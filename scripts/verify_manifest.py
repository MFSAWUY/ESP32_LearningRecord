"""Verify files copied or unpacked from the handbook source archive."""
import argparse
import hashlib
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    root = args.root.resolve()
    manifest = json.loads((root / "validation/source-manifest.json").read_text(encoding="utf-8"))
    errors = []
    for name, expected in manifest.items():
        path = (root / name).resolve()
        if not path.is_relative_to(root):
            errors.append(f"outside root: {name}")
        elif not path.is_file():
            errors.append(f"missing: {name}")
        elif hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            errors.append(f"changed: {name}")
    print(json.dumps(dict(files=len(manifest), errors=errors), ensure_ascii=False, indent=2))
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
