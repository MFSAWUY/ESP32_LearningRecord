"""Check native IDF images, generated partitions and flash ranges (never flash).

For build_idf_native.py use --build-root WORK_ROOT/builds. Without that flag,
inspect each example's build directory. --match selects a project substring.
Successful size checks do not establish OTA rollback or hardware operation.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path

from check_firmware_sizes import FLASH_BYTES, read_partitions

ROOT = Path(__file__).resolve().parents[1]


def inspect_build(build):
    errors = []
    args = json.loads((build / "flasher_args.json").read_text(encoding="utf-8"))
    if args["extra_esptool_args"]["chip"] != "esp32":
        errors.append("not the classic ESP32 target")
    if args["flash_settings"]["flash_size"] != "4MB":
        errors.append("flash size is not configured as 4MB")
    image = build / args["app"]["file"]
    size = image.stat().st_size
    parts = read_partitions(build / args["partition-table"]["file"])
    apps = [p for p in parts if p["type"] == 0]
    if not apps or any(size > p["size"] for p in apps):
        errors.append("image does not fit every application slot")
    ordered = sorted(parts, key=lambda p: p["offset"])
    for previous, following in zip(ordered, ordered[1:]):
        if previous["offset"] + previous["size"] > following["offset"]:
            errors.append("overlapping partitions")
    if any(p["size"] <= 0 or p["offset"] < 0 or p["offset"] + p["size"] > FLASH_BYTES for p in parts):
        errors.append("invalid partition range or partition exceeds 4 MiB")
    app_offset = int(args["app"]["offset"], 0)
    if not any(p["offset"] == app_offset for p in apps):
        errors.append("flash application offset does not match an application partition")
    flash_ranges = []
    for address, relative in args["flash_files"].items():
        binary = build / relative
        offset, length = int(address, 0), binary.stat().st_size
        if offset < 0 or length <= 0 or offset + length > FLASH_BYTES:
            errors.append(f"invalid flash range: {relative}")
        flash_ranges.append(dict(file=relative, offset=offset, size=length))
    flash_ranges.sort(key=lambda item: item["offset"])
    for previous, following in zip(flash_ranges, flash_ranges[1:]):
        if previous["offset"] + previous["size"] > following["offset"]:
            errors.append("flash files overlap")
    generated = json.loads((build / "config/sdkconfig.json").read_text(encoding="utf-8"))
    rollback = generated.get("BOOTLOADER_APP_ROLLBACK_ENABLE", False)
    return dict(status="failed" if errors else "passed", errors=errors,
                image_bytes=size, image_sha256=hashlib.sha256(image.read_bytes()).hexdigest(),
                smallest_app_free_bytes=min((p["size"] - size for p in apps), default=0),
                partitions=parts, flash_ranges=flash_ranges, rollback_enabled=rollback,
                ota_slots=sum(1 for p in apps if 0x10 <= p["subtype"] <= 0x1F))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=Path)
    parser.add_argument("--match", default="")
    parser.add_argument("--output", type=Path, default=ROOT / "validation/idf-sizes-current.json")
    options = parser.parse_args()
    results = []
    for cmake in sorted((ROOT / "examples/esp-idf").glob("*/CMakeLists.txt")):
        project = cmake.parent
        if options.match not in project.name:
            continue
        build = options.build_root / project.name if options.build_root else project / "build"
        try:
            record = inspect_build(build)
            if project.name == "ota" and (not record["rollback_enabled"] or record["ota_slots"] < 2):
                record["errors"].append("OTA requires rollback enabled and two application slots")
                record["status"] = "failed"
        except (OSError, ValueError, KeyError) as exc:
            record = dict(status="failed", errors=[str(exc)])
        results.append(dict(project=project.relative_to(ROOT).as_posix(), **record))
    report = dict(checked_utc=datetime.now(timezone.utc).isoformat(), flash_bytes=FLASH_BYTES,
                  hardware_tested=False, images=results)
    options.output.parent.mkdir(parents=True, exist_ok=True)
    options.output.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    passed = sum(r["status"] == "passed" for r in results)
    print(f"{passed}/{len(results)} native IDF images passed size and flash layout checks")
    return 0 if results and passed == len(results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
