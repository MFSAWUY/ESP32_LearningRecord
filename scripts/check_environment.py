"""Read-only host/port check; never installs drivers or modifies devices."""
import json
import platform
import shutil
import sys

result = {"system": platform.platform(), "python": sys.version, "executable": sys.executable,
          "tools": {name: shutil.which(name) for name in ["pio", "idf.py", "git", "cmake", "ninja"]}}
try:
    from serial.tools import list_ports
    result["ports"] = [{"device": port.device, "description": port.description} for port in list_ports.comports()]
except ImportError:
    result["ports"] = "pyserial not installed"
print(json.dumps(result, ensure_ascii=False, indent=2))
