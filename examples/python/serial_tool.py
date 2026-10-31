"""List serial ports, send a line and record bounded JSON telemetry to CSV."""
import argparse
import csv
from datetime import datetime, timezone
import json
from pathlib import Path
import sys
import time

from protocol import MAX_LINE_BYTES, parse_record


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ports", action="store_true")
    parser.add_argument("--port", help="COM number, /dev/ttyUSB*, or pyserial loop://")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--duration", type=float, default=30, help="seconds; 0 means until Ctrl+C")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--send", help="one UTF-8 line sent after opening the port")
    args = parser.parse_args()
    if args.baud <= 0 or args.duration < 0:
        parser.error("baud must be positive and duration non-negative")
    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        parser.error("install examples/python/requirements.txt first")
    if args.ports:
        ports = list(list_ports.comports())
        for port in ports:
            print(f"{port.device}\t{port.description}\t{port.hwid}")
        if not ports:
            print("No serial ports found; check cable, driver and permissions.")
        return 0
    if not args.port:
        parser.error("provide --ports or --port; no port is selected automatically")
    output = None
    try:
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            output = args.output.open("w", encoding="utf-8", newline="")
            writer = csv.DictWriter(output, fieldnames=["received_utc", "seq", "uptime_ms", "temperature_c", "humidity_pct", "valid", "raw_json"])
            writer.writeheader()
        with serial.serial_for_url(args.port, baudrate=args.baud, timeout=0.5, write_timeout=2) as connection:
            if args.send is not None:
                connection.write((args.send.rstrip("\r\n") + "\n").encode("utf-8"))
                connection.flush()
            started = time.monotonic()
            pending = bytearray()
            discarding = False
            while args.duration == 0 or time.monotonic() - started < args.duration:
                for byte in connection.read(connection.in_waiting or 1):
                    if byte == 10:
                        if discarding:
                            discarding = False
                            pending.clear()
                            continue
                        line = bytes(pending).rstrip(b"\r")
                        pending.clear()
                        if not line:
                            continue
                        try:
                            record = parse_record(line)
                        except ValueError as exc:
                            print(f"[non-telemetry] {exc}: {line[:160]!r}", file=sys.stderr)
                            continue
                        encoded = json.dumps(record, ensure_ascii=False, allow_nan=False)
                        print(encoded, flush=True)
                        if output:
                            row = {key: record.get(key, "") for key in writer.fieldnames}
                            row.update(received_utc=datetime.now(timezone.utc).isoformat(), raw_json=encoded)
                            writer.writerow(row)
                            output.flush()
                    elif not discarding:
                        pending.append(byte)
                        if len(pending) > MAX_LINE_BYTES:
                            print("[discard] overlong frame until next newline", file=sys.stderr)
                            pending.clear()
                            discarding = True
        return 0
    except KeyboardInterrupt:
        return 0
    except (serial.SerialException, OSError) as exc:
        print(f"Serial/file error: {exc}", file=sys.stderr)
        return 1
    finally:
        if output:
            output.close()


if __name__ == "__main__":
    raise SystemExit(main())
