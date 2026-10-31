"""Check the 72 chapter inventory, local links, fences and Python syntax."""
import ast
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]


def main():
    errors, warnings = [], []
    chapters = sorted((ROOT / "docs").rglob("*.md"))
    numbers = [int(p.name[:2]) for p in chapters if re.match(r"\d\d-", p.name)]
    if sorted(numbers) != list(range(1, 73)):
        errors.append(f"Expected chapters 01-72 exactly once; found {len(numbers)}: {numbers}")
    markdown = [ROOT / "README.md", *chapters, *list((ROOT / "examples").rglob("README.md")), *list((ROOT / "validation").glob("*.md"))]
    markdown = [p for p in markdown if not any(x in p.parts for x in (".pio", "build", "managed_components"))]
    count = 0
    for path in markdown:
        if not path.exists():
            errors.append(f"Missing {path.relative_to(ROOT)}")
            continue
        content = path.read_text(encoding="utf-8")
        count += len(content)
        if path in chapters and len(content) < 800:
            warnings.append(f"Short chapter: {path.relative_to(ROOT)} ({len(content)} chars)")
        if len(re.findall(r"^\s*```", content, flags=re.M)) % 2:
            errors.append(f"Unbalanced code fence: {path.relative_to(ROOT)}")
        prose = re.sub(r"```.*?```", "", content, flags=re.S)
        for match in re.finditer(r"!?\[[^\]]*\]\(([^)]+)\)", prose):
            href = match.group(1).strip().strip("<>")
            if re.match(r"[a-zA-Z][a-zA-Z0-9+.-]*:", href) or href.startswith("#"):
                continue
            target = unquote(href.split("#", 1)[0].split("?", 1)[0])
            if target and not (path.parent / target).exists():
                errors.append(f"Broken link: {path.relative_to(ROOT)} -> {href}")
    for path in [*list((ROOT / "scripts").rglob("*.py")), *list((ROOT / "examples/python").rglob("*.py"))]:
        try:
            ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
        except (SyntaxError, UnicodeError) as exc:
            errors.append(str(exc))
    result = {"chapters": len(chapters), "markdown_characters": count, "errors": errors, "warnings": warnings}
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return bool(errors)


if __name__ == "__main__":
    raise SystemExit(main())
