"""Check HTTP references; blocked/timeouts are inconclusive, not broken links."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import re
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]


def check(url):
    result = {"url": url}
    try:
        request = Request(url, headers={"User-Agent": "ESP32-Handbook-LinkCheck/1.0"})
        with urlopen(request, timeout=20) as response:
            result.update(status="reachable", http=response.status, final_url=response.url)
    except HTTPError as exc:
        result.update(status="missing" if exc.code in (404, 410) else "inconclusive", http=exc.code)
    except (URLError, TimeoutError, OSError) as exc:
        result.update(status="inconclusive", error=str(exc))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "validation/external-links.json")
    args = parser.parse_args()
    urls = set()
    for folder in (ROOT / "docs", ROOT / "examples"):
        for path in folder.rglob("*.md"):
            if set(path.parts) & {".pio", "build", "managed_components", ".tools"}:
                continue
            for url in re.findall(r"\]\((https?://[^\s)]+)\)", path.read_text(encoding="utf-8")):
                urls.add(url.split("#", 1)[0])
    with ThreadPoolExecutor(max_workers=6) as pool:
        results = list(pool.map(check, sorted(urls)))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({status: sum(r["status"] == status for r in results)
                      for status in ("reachable", "missing", "inconclusive")}))
    return int(any(r["status"] == "missing" for r in results))


if __name__ == "__main__":
    raise SystemExit(main())
