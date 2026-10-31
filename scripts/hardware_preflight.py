"""Read-only ESP32 hardware inventory for Windows/Linux; never opens a port.

Examples:
  python scripts/hardware_preflight.py --usb --output validation/preflight.json
  python scripts/hardware_preflight.py --port ACTUAL_PORT --require-port

Exit 0 means inventory completed, NOT that any hardware experiment passed.
Exit 1 means serial inventory failed; exit 2 means --require-port was unmet.
USB inventory is optional and failures are recorded without hiding serial data.
"""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys


def serial_inventory():
    try:
        from serial.tools import list_ports
    except ImportError:
        return [], "pyserial unavailable: install examples/python/requirements.txt"
    try:
        ports = []
        for port in sorted(list_ports.comports(), key=lambda item: item.device):
            ports.append({
                "device": port.device,
                "description": port.description,
                "hwid": port.hwid,
                "vid": f"{port.vid:04X}" if port.vid is not None else None,
                "pid": f"{port.pid:04X}" if port.pid is not None else None,
                "manufacturer": port.manufacturer,
                "serial_number": port.serial_number,
                "location": port.location,
            })
        return ports, None
    except (OSError, RuntimeError) as error:
        return [], f"{type(error).__name__}: {error}"


def usb_inventory():
    """Query OS inventory only. No USB claims, transfers, driver installation."""
    system = platform.system()
    if system == "Windows":
        executable = shutil.which("powershell.exe") or shutil.which("pwsh")
        if not executable:
            return {"status": "unavailable", "reason": "PowerShell not on PATH"}
        query = (
            "[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new(); "
            "$ErrorActionPreference = 'Stop'; "
            "@(Get-PnpDevice -PresentOnly | Where-Object { "
            "$_.InstanceId -like 'USB\\VID_*' } | "
            "Select-Object Status, Class, FriendlyName, InstanceId) | "
            "ConvertTo-Json -Depth 3 -Compress"
        )
        command = [executable, "-NoProfile", "-NonInteractive", "-Command", query]
    elif system == "Linux":
        executable = shutil.which("lsusb")
        if not executable:
            return {"status": "unavailable", "reason": "lsusb absent (Ubuntu package: usbutils)"}
        command = [executable]
    else:
        return {"status": "unsupported", "reason": f"USB inventory not implemented for {system}"}
    try:
        result = subprocess.run(command, capture_output=True, text=True,
                                encoding="utf-8", errors="replace", timeout=20)
        if result.returncode:
            return {"status": "unavailable", "exit_code": result.returncode,
                    "reason": result.stderr.strip()[:2000]}
        if system == "Windows":
            devices = json.loads(result.stdout.lstrip("\ufeff").strip() or "[]")
            if isinstance(devices, dict):
                devices = [devices]
            return {"status": "enumerated", "devices": devices}
        return {"status": "enumerated", "lines": result.stdout.splitlines()}
    except (OSError, subprocess.TimeoutExpired, json.JSONDecodeError) as error:
        return {"status": "unavailable", "reason": f"{type(error).__name__}: {error}"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="Compare this exact enumerated port; do not open it")
    parser.add_argument("--usb", action="store_true", help="Also read the host OS USB inventory")
    parser.add_argument("--output", type=Path, help="Save JSON; stdout always receives the same report")
    parser.add_argument("--require-port", action="store_true", help="Exit 2 if no port / selected port absent")
    args = parser.parse_args()
    ports, inventory_error = serial_inventory()
    normalize = str.casefold if platform.system() == "Windows" else str
    selected = next((p for p in ports if args.port and normalize(p["device"]) == normalize(args.port)), None)
    serial_requirement = bool(selected) if args.port else bool(ports)
    report = {
        "schema_version": 1,
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "host": platform.platform(),
        "python": sys.version.split()[0],
        "python_executable": sys.executable,
        "hardware_tested": False,
        "opened_serial_ports": False,
        "firmware_flashed": False,
        "serial_inventory": {"status": "failed" if inventory_error else "enumerated",
                             "ports": ports, "error": inventory_error},
        "selected_port": args.port,
        "selected_port_present": bool(selected) if args.port else None,
        "usb_inventory": usb_inventory() if args.usb else {"status": "not_requested"},
        "tools_on_path": {tool: shutil.which(tool) for tool in
                          ("git", "pio", "platformio", "idf.py", "cmake", "ninja",
                           "openocd", "xtensa-esp-elf-gdb")},
        "idf_environment_selected": bool(os.environ.get("IDF_PATH")),
        "limitations": [
            "A USB UART bridge or serial port does not prove an ESP32 is connected.",
            "Enumeration does not verify port permissions, baud rate, firmware or flash size.",
            "SSH does not forward a Windows USB device to the remote Linux host.",
            "CH340 is a USB UART bridge, not the classic ESP32 JTAG interface.",
            "No OLED, sensor, OTA, power or JTAG experiment is performed by this script.",
        ],
        "experiments": {
            "serial": {"status": "not_run", "needs": ["identified board", "data USB cable", "selected usable port"]},
            "oled": {"status": "not_run", "needs": ["I2C wiring", "address scan", "controller/resolution confirmation", "visual inspection"]},
            "sht30": {"status": "not_run", "needs": ["physical 3.3 V compatible SHT30 module", "CRC and recovery logs"]},
            "ota_rollback": {"status": "not_run", "needs": ["confirmed baseline image", "two OTA slots", "trusted HTTPS server", "good and failing updates"]},
            "power": {"status": "not_run", "needs": ["current measurement instrument", "documented measurement point", "active/sleep waveform or samples"]},
            "jtag": {"status": "not_run", "needs": ["external ESP32 compatible JTAG probe", "correct drivers", "matching ELF", "breakpoint/backtrace evidence"]},
        },
    }
    code = 1 if inventory_error else 2 if args.require_port and not serial_requirement else 0
    report["exit_code"] = code
    report["exit_meaning"] = "inventory completed; no physical experiment passed" if code == 0 else "serial inventory failed" if code == 1 else "required serial port absent"
    content = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(content, encoding="utf-8")
    # Keep stdout safe on Windows terminals whose code page cannot display a device name.
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    print(content, end="")
    return code


if __name__ == "__main__":
    raise SystemExit(main())
