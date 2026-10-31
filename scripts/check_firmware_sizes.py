"""Check actual Arduino binaries against their generated ESP32 partition tables.

Run after building: python scripts/check_firmware_sizes.py
Use --build-root if build_examples.py used a separate build directory.
This checks image sizes, not successful OTA, booting, signing or hardware.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
FLASH_BYTES = 4 * 1024 * 1024


def read_partitions(path):
    data = path.read_bytes()
    result = []
    for start in range(0, len(data) - 31, 32):
        magic, kind, subtype, offset, size, label, flags = struct.unpack_from("<HBBII16sI", data, start)
        if magic in (0xFFFF, 0xEBEB):  # erased entries or MD5 trailer
            break
        if magic != 0x50AA:
            raise ValueError(f"bad partition magic at {start}")
        result.append(dict(name=label.split(b"\0")[0].decode("ascii"), type=kind,
                           subtype=subtype, offset=offset, size=size, flags=flags))
    if not result:
        raise ValueError("empty partition table")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=Path)
    parser.add_argument("--output", type=Path, default=ROOT / "validation/firmware-sizes.json")
    args = parser.parse_args()
    results = []
    missing = []
    for category in ("arduino", "projects"):
        for ini in sorted((ROOT / "examples" / category).glob("*/platformio.ini")):
            build = (args.build_root / category / ini.parent.name / "build") if args.build_root else ini.parent / ".pio/build"
            images = sorted(build.glob("*/firmware.bin"))
            if not images:
                missing.append(ini.parent.relative_to(ROOT).as_posix())
            for firmware in images:
                partitions = read_partitions(firmware.parent / "partitions.bin")
                applications = [p for p in partitions if p["type"] == 0]
                size = firmware.stat().st_size
                errors = []
                if not applications or any(size > p["size"] for p in applications):
                    errors.append("image does not fit every application slot")
                ordered = sorted(partitions, key=lambda p: p["offset"])
                for previous, following in zip(ordered, ordered[1:]):
                    if previous["offset"] + previous["size"] > following["offset"]:
                        errors.append("overlapping partitions")
                if any(p["offset"] + p["size"] > FLASH_BYTES for p in partitions):
                    errors.append("partition exceeds 4 MiB")
                results.append(dict(project=ini.parent.relative_to(ROOT).as_posix(),
                                    environment=firmware.parent.name, image_bytes=size,
                                    image_sha256=hashlib.sha256(firmware.read_bytes()).hexdigest(),
                                    partitions=partitions,
                                    smallest_app_free_bytes=min((p["size"] - size for p in applications), default=0),
                                    status="failed" if errors else "passed", errors=errors))
    report = dict(flash_bytes=FLASH_BYTES, hardware_tested=False, images=results, unbuilt_projects=missing)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"{len(results)} images checked; {len(missing)} projects without binaries")
    return 0 if results and not missing and all(r["status"] == "passed" for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
