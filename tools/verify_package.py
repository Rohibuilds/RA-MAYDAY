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
require("#define GPS_RX       16" in sender and "#define GPS_TX       17" in sender, "sender NEO-6M UART pin map matches documented wiring")
require("#define NRF_CE        4" in sender and "#define NRF_CSN       5" in sender and "#define NRF_SCK      12" in sender and "#define NRF_MISO     13" in sender and "#define NRF_MOSI     11" in sender, "sender nRF24 SPI pin map matches documented wiring")
require("airHealthy ? temperature : NAN" in sender and "airHealthy ? humidity : NAN" in sender and "pressureHealthy ? pressure : NAN" in sender, "unavailable sender sensor values are marked invalid")
require("sizeof(SurvivorPacket)==32" in receiver.replace(" ", ""), "receiver packet layout remains 32 bytes")
require("RF24_PA_HIGH" in receiver and "RADIO_CHANNEL=76" in receiver and "MAY01" in receiver, "receiver PA+LNA ACK and radio settings are explicit")
require("NRF_CE=25" in receiver and "NRF_CSN=26" in receiver and "NRF_SCK=27" in receiver and "NRF_MISO=19" in receiver and "NRF_MOSI=23" in receiver, "receiver nRF24 SPI pin map is explicit")
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
