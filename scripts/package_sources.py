"""Package only handbook sources; exclude tools, build products and local secrets."""
import argparse
import hashlib
import json
from pathlib import Path
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED_PARTS = {".pio", "build", "managed_components", "__pycache__", ".venv", ".tools", ".git"}
EXCLUDED_NAMES = {"config.local.h", "secrets.h", "tls_config.h", "server_root.h", "sdkconfig", "sdkconfig.old", "sdkconfig.local"}


def sources():
    items = [ROOT / "README.md", ROOT / "LICENSE", ROOT / ".gitignore", ROOT / ".gitattributes"]
    for folder in ["docs", "examples", "assets", "scripts", "validation", ".github"]:
        items.extend((ROOT / folder).rglob("*"))
    return sorted(p for p in items if p.is_file() and not (set(p.relative_to(ROOT).parts) & EXCLUDED_PARTS)
                  and p.name not in EXCLUDED_NAMES and p.suffix not in (".pyc", ".key", ".pem")
                  and p.name != "source-manifest.json")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "ESP32-handbook.zip")
    args = parser.parse_args()
    paths = sources()
    manifest = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    manifest_path = ROOT / "validation/source-manifest.json"
    manifest_path.parent.mkdir(exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2), encoding="utf-8")
    paths.append(manifest_path)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if str(args.output).endswith(".tar.gz"):
        with tarfile.open(args.output, "w:gz") as archive:
            for path in paths:
                archive.add(path, arcname=path.relative_to(ROOT).as_posix(), recursive=False)
    else:
        with zipfile.ZipFile(args.output, "w", zipfile.ZIP_DEFLATED) as archive:
            for path in paths:
                archive.write(path, arcname=path.relative_to(ROOT).as_posix())
    print(f"{len(paths)} files -> {args.output}")


if __name__ == "__main__":
    main()
