# RA MAYDAY — Tactical Receiver v3.0

This replaces the firmware on your **classic ESP32 receiver**, not the ESP32-S3 sender.
No AWS, Pi, internet, CloudShell, router, or filesystem uploader is needed.

## 1. Wiring — receiver only

Keep power off while checking connections.

| nRF24 pin | Classic ESP32 GPIO |
|---|---:|
| CE | 25 |
| CSN | 26 |
| SCK | 27 |
| MISO | 19 |
| MOSI | 23 |
| VCC | Stable 3.3 V |
| GND | GND |

Keep the existing correctly wired nRF24 supply and nearby decoupling capacitor.
Do not connect the nRF24 directly to 5 V. PA/LNA modules may need a dedicated regulator.

## 2. Open and upload

1. Extract the complete project.
2. Open `firmware/RA_MAYDAY_Offline/RA_MAYDAY_Offline.ino` in Arduino IDE.
3. Keep `Dashboard.h` and `CommandBackground.h` beside the receiver sketch.
4. Install **esp32 by Espressif Systems** in Boards Manager.
5. Install **RF24 by TMRh20** in Library Manager.
6. Select **ESP32 Dev Module**, select the receiver's port, and upload.
7. Open Serial Monitor at **115200 baud**. Look for `[WEB] http://192.168.4.1` and `[NRF24] READY`.

WiFi, WebServer and SPI come with the Espressif board package. Website HTML, JavaScript,
CSS and vector background are compiled into flash; no filesystem upload is needed.

## 3. Connect

- Wi-Fi: **RA-MAYDAY**
- Example password: **Mayday2026** (case-sensitive)
- Dashboard: **http://192.168.4.1/**

Stay connected if the phone reports "No internet". Change the example AP password before
field use. Any joined client can read telemetry.

## 4. Verify telemetry

The receiver uses the existing 32-byte fixed payload, pipe MAY01, channel 76, 250 kbps,
CRC16 and automatic acknowledgement. Check that sequence numbers increase and the
survivor ID appears. Test SOS press/release transitions. Turn off the sender and confirm
stale status. GPS NO FIX is expected until the sender obtains a valid fix.

## 5. Teams and ETA

Create a team, enter its name, members, travel speed and (if known) coordinates. Assign
a received survivor ID, priority, preparation time and notes. Update mission state
manually: Assigned → En route → On scene → Rescued.

Teams, assignments, incidents and notes are stored in the current browser. They are
not transmitted to responder radios or synchronized between browsers. Confirm dispatch
over your actual field communications channel.

ETA is straight-line distance divided by entered speed, plus preparation time and the
configured planning multiplier. It is not a road route, safe-path, extraction-time or
survival prediction. It requires a fresh survivor GPS fix and valid entered team position.

## 6. Map and data limits

- The tactical map plots received GPS coordinates and trails.
- HQ, search radius and checkpoints are operator-entered planning overlays.
- The local coordinate map is not an offline street map, surveyed terrain map,
  obstacle-aware route planner or radio coverage map.
- Eight survivor display slots do not guarantee collision-free RF reception.
- Receiver packet/event history is RAM-only and resets on reboot.
- The current 32-byte packet does not contain battery, gas, breathing, indoor 3D position,
  altitude or satellite count. These are not fabricated in the UI.
- Sender-local experimental Wi-Fi CSI is not transported to the receiver.

## 7. Build and verify after dashboard edits

After editing `web/receiver/dashboard.html` or `command-grid.svg`, run:

```bash
python3 tools/build_dashboard.py
python3 tools/verify_package.py
```

These checks do not replace an Arduino board build, physical RF test, browser visual check
or field validation. This is an experimental prototype, not a certified life-safety system.
