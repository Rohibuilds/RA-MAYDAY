# RA MAYDAY · COLOSSUS 3.0

Offline rescue-operations prototype for an ESP32-S3 survivor sender and a classic ESP32 receiver. The receiver hosts the command dashboard over its own Wi-Fi access point; no cloud, Raspberry Pi, or internet connection is required for the core workflow.

## Included

- `firmware/RA_MAYDAY_Sender/` — ESP32-S3 sender firmware: NEO-6M GPS, AHT20/BMP280 environmental telemetry, nRF24L01+ radio, SOS button, OLED, WS2812 and a local field portal.
- `firmware/RA_MAYDAY_Offline/` — classic ESP32 receiver firmware and embedded dashboard assets.
- `web/receiver/` — editable receiver dashboard and matching command-grid SVG.
- `docs/START_HERE.md` — detailed wiring, upload, setup and operational notes.
- `tools/build_dashboard.py` — regenerate embedded HTML/SVG headers after web edits.
- `tools/verify_package.py` — portable source integrity and JavaScript syntax checks.

## Dashboard capabilities

Survivor registry (up to eight display slots), received GPS positions and trails, animated tactical plotting, SOS event history, stale-data detection, temperature/humidity/pressure trends, packet search and CSV export, HQ and search-radius planning, checkpoints, GeoJSON/map-image export, team creation and assignment, priority and mission-state tracking, operator notes/log, direct distance and bearing, planning ETA, incident desk, optional audible SOS alert, receiver diagnostics, settings, backup/restore, print and fullscreen controls.

The sender and receiver now use a coordinated graphite-and-amber command-console visual theme. The dashboard uses the same local visual language on both device portals.

## Quick start

1. Read `docs/START_HERE.md` completely and verify wiring with power disconnected.
2. Install Espressif ESP32 board support and RF24 by TMRh20. The sender additionally requires TinyGPSPlus, Adafruit AHTX0, Adafruit BMP280, Adafruit GFX, Adafruit SSD1306 and Adafruit NeoPixel.
3. Upload the sender sketch to the ESP32-S3 board and the receiver sketch to the classic ESP32 board. Keep `Dashboard.h` and `CommandBackground.h` beside the receiver sketch.
4. Join receiver Wi-Fi `RA-MAYDAY` (example password `Mayday2026`) and open `http://192.168.4.1/`.
5. Change example Wi-Fi credentials before deployment.

After editing `web/receiver/dashboard.html` or `command-grid.svg`, run:

```bash
python3 tools/build_dashboard.py
python3 tools/verify_package.py
```

## Data and operational limits

The radio packet is a fixed 32-byte payload: sequence, GPS latitude/longitude, temperature, humidity, pressure, flags and survivor ID. It does not contain battery, gas, respiration, indoor 3D position, altitude or satellite count. These are not fabricated in the UI. Sender-local experimental Wi-Fi CSI is not sent to the receiver.

The tactical map is a local GPS coordinate plot with real received positions/trails and operator-entered HQ/checkpoints/teams. It is not an offline street map, surveyed terrain map, obstacle-aware route planner or radio coverage map. Search radius is an operator planning boundary. ETA is straight-line distance divided by the operator-entered team speed, plus preparation time and configured multiplier; it is not a safe-route, extraction-time or survival prediction.

Team locations, assignments, incidents and settings are stored in the current browser. They are not transmitted to responder radios and do not synchronize across operators. Receiver packet/event history is held in RAM and resets on reboot. Confirm dispatch over the actual field communications channel.

This is an experimental prototype, not a certified life-safety system. Validate the hardware, radio range, power, weather protection, GPS accuracy, browser behavior and field procedures before any real-world use. Do not rely on this device as the sole emergency communications or rescue system.

## Quality and field validation

- GitHub Actions runs package-integrity and dashboard JavaScript checks on pushes and pull requests: `.github/workflows/quality.yml`.
- Follow `docs/FIELD_VALIDATION.md` for board-specific firmware builds, radio/GPS bench tests, range/interference tests, dashboard checks and supervised field-readiness sign-off.
- A passing source workflow does not prove firmware compilation, radio performance, GPS accuracy or life-safety suitability.
