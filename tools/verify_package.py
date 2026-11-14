#!/usr/bin/env python3
"""Run portable source-package integrity checks (not a board/hardware test)."""
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SENDER = ROOT / "firmware/RA_MAYDAY_Sender/RA_MAYDAY_Sender.ino"
RECEIVER = ROOT / "firmware/RA_MAYDAY_Offline/RA_MAYDAY_Offline.ino"
DASHBOARD = ROOT / "web/receiver/dashboard.html"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("FAIL: " + message)
    print("PASS: " + message)


require(SENDER.exists() and RECEIVER.exists(), "both sender and receiver sketches are present")
sender = SENDER.read_text(encoding="utf-8")
receiver = RECEIVER.read_text(encoding="utf-8")
require("sizeof(SurvivorPacket) == 32" in sender, "sender radio payload remains 32 bytes")
require("sizeof(SurvivorPacket)==32" in receiver.replace(" ", ""), "receiver packet layout remains 32 bytes")
require('"/api/telemetry"' in receiver, "receiver telemetry endpoint is registered")
require("/api/telemetry" in DASHBOARD.read_text(encoding="utf-8"), "dashboard polls receiver telemetry")
subprocess.run([sys.executable, str(ROOT / "tools/build_dashboard.py"), "--check"], check=True)
node = shutil.which("node")
if node:
    source = DASHBOARD.read_text(encoding="utf-8")
    match = re.search(r"<script>(.*?)</script>", source, re.S)
    require(match is not None, "dashboard JavaScript block exists")
    temp = ROOT / ".dashboard-check.js"
    try:
        temp.write_text(match.group(1), encoding="utf-8")
        subprocess.run([node, "--check", str(temp)], check=True)
        print("PASS: dashboard JavaScript syntax")
    finally:
        temp.unlink(missing_ok=True)
else:
    print("SKIP: Node.js not installed; dashboard syntax check not run")
print("NOTE: this does not compile Arduino firmware or validate RF hardware / field safety.")
