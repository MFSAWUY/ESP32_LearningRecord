"""Build independent ESP-IDF projects with official idf.py in isolated copies.

Run inside an activated ESP-IDF terminal. --work-root may be a short Windows
directory to avoid command-line length limits. No firmware is uploaded.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work-root", type=Path, required=True)
    parser.add_argument("--idf-path", type=Path, default=os.environ.get("IDF_PATH"))
    parser.add_argument("--python", default=sys.executable, help="Python from the IDF environment")
    parser.add_argument("--output", type=Path, default=ROOT / "validation" / f"idf-native-{platform.system().lower()}.json")
    parser.add_argument("--match", default="")
    parser.add_argument("--parallel", type=int, choices=(1, 2), default=1)
    parser.add_argument("--timeout", type=int, default=1200)
    parser.add_argument("--retry-failed", action="store_true")
    parser.add_argument("--defaults-overlay", type=Path, help="additional public build-only sdkconfig defaults; use a fresh work root")
    args = parser.parse_args()
    if not args.idf_path or not (args.idf_path / "tools/idf.py").is_file():
        parser.error("activate the IDF environment or provide --idf-path")
    records = json.loads(args.output.read_text(encoding="utf-8")) if args.retry_failed and args.output.exists() else []
    args.output.parent.mkdir(parents=True, exist_ok=True)
    logdir = args.output.parent / "logs"
    logdir.mkdir(exist_ok=True)

    def build(original):
        name = original.relative_to(ROOT).as_posix()
        target = args.work_root.absolute() / "sources" / original.name
        build_dir = args.work_root.absolute() / "builds" / original.name
        hashes = {}
        for source in original.rglob("*"):
            relative = source.relative_to(original)
            if not source.is_file() or set(relative.parts) & {"build", ".pio", "managed_components", ".git"}:
                continue
            if source.name in {"sdkconfig", "sdkconfig.old", "server_root.h", "sdkconfig.local"}:
                continue
            destination = target / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, destination)
            hashes[relative.as_posix()] = hashlib.sha256(source.read_bytes()).hexdigest()
        command = [args.python, str(args.idf_path / "tools/idf.py"), "-C", str(target),
                   "-B", str(build_dir), "-D", "IDF_TARGET=esp32", "build"]
        if args.defaults_overlay:
            overlay = target / "validation.defaults"
            shutil.copy2(args.defaults_overlay, overlay)
            command[-1:-1] = ["-D", "SDKCONFIG_DEFAULTS=sdkconfig.defaults;validation.defaults"]
        log = logdir / f"idf-native-{platform.system().lower()}-{original.name}.log"
        record = dict(project=name, host=platform.platform(), method="official idf.py; isolated source copy",
                      started_utc=datetime.now(timezone.utc).isoformat(), command=command,
                      source_sha256=hashes, hardware_tested=False, log=log.name)
        if args.defaults_overlay:
            record["defaults_overlay_sha256"] = hashlib.sha256(args.defaults_overlay.read_bytes()).hexdigest()
        print(f"BUILD {name}", flush=True)
        environment = os.environ.copy()
        environment["PYTHONUTF8"] = "1"
        environment.setdefault("IDF_PY_BUILD_JOBS", "4")
        try:
            with log.open("wb") as stream:
                result = subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT,
                                        timeout=args.timeout, env=environment, cwd=target)
            record.update(status="passed" if result.returncode == 0 else "failed", exit_code=result.returncode)
            image = build_dir / (original.name + ".bin")
            if result.returncode == 0 and image.exists():
                record.update(image_bytes=image.stat().st_size,
                              image_sha256=hashlib.sha256(image.read_bytes()).hexdigest())
        except (FileNotFoundError, subprocess.TimeoutExpired) as exc:
            record.update(status="unavailable" if isinstance(exc, FileNotFoundError) else "timeout", error=str(exc))
        print(f"{name}: {record['status']}", flush=True)
        return record

    pending = []
    for cmake in sorted((ROOT / "examples/esp-idf").glob("*/CMakeLists.txt")):
        original = cmake.parent
        name = original.relative_to(ROOT).as_posix()
        if args.match not in name:
            continue
        old = next((r for r in records if r["project"] == name), None)
        if args.retry_failed and old and old["status"] == "passed":
            continue
        pending.append(original)
    with ThreadPoolExecutor(max_workers=args.parallel) as pool:
        for future in as_completed([pool.submit(build, original) for original in pending]):
            record = future.result()
            records = [r for r in records if r["project"] != record["project"]] + [record]
            args.output.write_text(json.dumps(records, ensure_ascii=False, indent=2), encoding="utf-8")
    return 0 if records and all(r["status"] == "passed" for r in records) else 1


if __name__ == "__main__":
    raise SystemExit(main())
