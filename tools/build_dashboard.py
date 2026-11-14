#!/usr/bin/env python3
"""Embed editable receiver web assets into Arduino PROGMEM headers."""
from pathlib import Path
import argparse

ROOT = Path(__file__).resolve().parents[1]
HTML = ROOT / "web/receiver/dashboard.html"
SVG = ROOT / "web/receiver/command-grid.svg"
OUT_HTML = ROOT / "firmware/RA_MAYDAY_Offline/Dashboard.h"
OUT_SVG = ROOT / "firmware/RA_MAYDAY_Offline/CommandBackground.h"


def render(source: str, symbol: str, delimiter: str, comment: str) -> str:
    if f"){delimiter}\"" in source:
        raise SystemExit(f"Raw-string delimiter collision: {delimiter}")
    return ("#pragma once\n#include <Arduino.h>\n"
            f"const char {symbol}[] PROGMEM = R\"{delimiter}({source}){delimiter}\";\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail if embedded headers are out of date")
    args = parser.parse_args()
    expected = {
        OUT_HTML: render(HTML.read_text(encoding="utf-8"), "DASHBOARD_HTML", "MAYDAY", "HTML"),
        OUT_SVG: render(SVG.read_text(encoding="utf-8"), "COMMAND_BACKGROUND", "ART", "SVG"),
    }
    stale = []
    for path, content in expected.items():
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            stale.append(path.relative_to(ROOT).as_posix())
            if not args.check:
                path.write_text(content, encoding="utf-8")
                print(f"Updated {path.relative_to(ROOT)}")
    if args.check and stale:
        print("Embedded dashboard assets are stale: " + ", ".join(stale))
        print("Run: python3 tools/build_dashboard.py")
        return 1
    if args.check:
        print("Embedded dashboard assets are in sync.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
