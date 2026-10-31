"""Build independent projects, record actual results; never uploads firmware."""
import argparse
from datetime import datetime, timezone
import json
import hashlib
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kind", choices=["arduino", "idf", "all"], default="arduino")
    parser.add_argument("--match", default="", help="project path substring")
    parser.add_argument("--output", type=Path, default=ROOT / "validation" / f"build-{platform.system().lower()}.json")
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--build-root", type=Path, help="optional separate directory for build products")
    parser.add_argument("--retry-failed", action="store_true", help="keep prior successes, retry failures and new projects")
    args = parser.parse_args()
    projects = []
    if args.kind in ("arduino", "all"):
        projects += sorted((ROOT / "examples/arduino").glob("*/platformio.ini"))
        projects += sorted((ROOT / "examples/projects").glob("*/platformio.ini"))
    if args.kind in ("idf", "all"):
        projects += sorted((ROOT / "examples/esp-idf").glob("*/CMakeLists.txt"))
    records = json.loads(args.output.read_text(encoding="utf-8")) if args.retry_failed and args.output.exists() else []
    for manifest in projects:
        directory = manifest.parent
        name = directory.relative_to(ROOT).as_posix()
        if args.match not in name:
            continue
        old = next((record for record in records if record["project"] == name), None)
        if args.retry_failed and old and old["status"] == "passed":
            continue
        records = [record for record in records if record["project"] != name]
        command = [sys.executable, "-m", "platformio", "run", "-d", str(directory)] if manifest.suffix == ".ini" else ["idf.py", "-C", str(directory), "-D", "IDF_TARGET=esp32", "build"]
        environment = os.environ.copy()
        if args.build_root:
            external = args.build_root.resolve() / directory.parent.name / directory.name
            if manifest.suffix == ".ini":
                environment["PLATFORMIO_BUILD_DIR"] = str(external / "build")
                environment["PLATFORMIO_LIBDEPS_DIR"] = str(external / "libdeps")
            else:
                command[3:3] = ["-B", str(external / "build")]
        logdir = args.output.parent / "logs"
        logdir.mkdir(parents=True, exist_ok=True)
        log = logdir / (platform.system().lower() + "-" + directory.name + ".log")
        source_hashes = {}
        for source in directory.rglob("*"):
            relative = source.relative_to(directory)
            if source.is_file() and not set(relative.parts) & {".pio", "build", "managed_components", "__pycache__"} and source.suffix in {".c", ".cpp", ".h", ".ini", ".csv", ".txt"}:
                if source.name not in {"config.local.h", "server_root.h", "secrets.h"}:
                    source_hashes[relative.as_posix()] = hashlib.sha256(source.read_bytes()).hexdigest()
        record = {"project": name, "host": platform.platform(), "command": command,
                  "source_sha256": source_hashes,
                  "started_utc": datetime.now(timezone.utc).isoformat(), "hardware_tested": False}
        print(f"BUILD {name}", flush=True)
        try:
            result = subprocess.run(command, cwd=ROOT, capture_output=True, timeout=args.timeout, env=environment)
            output = result.stdout + b"\n" + result.stderr
            log.write_bytes(output)
            record.update(status="passed" if result.returncode == 0 else "failed", exit_code=result.returncode, log=log.name)
        except FileNotFoundError as exc:
            record.update(status="unavailable", error=str(exc))
        except subprocess.TimeoutExpired as exc:
            log.write_bytes((exc.stdout or b"") + (exc.stderr or b""))
            record.update(status="timeout", log=log.name)
        records.append(record)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(records, ensure_ascii=False, indent=2), encoding="utf-8")
        print(record["status"], flush=True)
    return 0 if records and all(r["status"] == "passed" for r in records) else 1


if __name__ == "__main__":
    raise SystemExit(main())
