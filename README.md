# RA MAYDAY · COLOSSUS 3.0

Offline rescue-operations prototype for an ESP32-S3 survivor sender and a classic ESP32
receiver. The receiver hosts its command dashboard over its own Wi-Fi access point.

## Repository publication status

This repository currently contains the receiver firmware entry point, command background
asset, setup guide and build/verification utilities. The complete source package supplied
separately also contains the sender sketch, editable dashboard HTML and embedded headers;
those larger files are **not yet committed here**. Do not treat this repository checkout
as a complete build until those source files are added.

## Files currently published

- `firmware/RA_MAYDAY_Offline/RA_MAYDAY_Offline.ino` — classic ESP32 receiver.
- `web/receiver/command-grid.svg` — graphite/amber command-console background.
- `docs/START_HERE.md` — receiver wiring, upload and operating guide.
- `tools/build_dashboard.py` — regenerates embedded HTML/SVG headers from editable web assets.
- `tools/verify_package.py` — portable source integrity checks.

## Planned complete package structure

```text
firmware/
  RA_MAYDAY_Sender/RA_MAYDAY_Sender.ino
  RA_MAYDAY_Offline/
    RA_MAYDAY_Offline.ino
    Dashboard.h
    CommandBackground.h
web/receiver/
  dashboard.html
  command-grid.svg
docs/START_HERE.md
tools/
  build_dashboard.py
  verify_package.py
```

## Receiver architecture

The receiver uses an nRF24L01+ link on channel 76, 250 kbps, pipe `MAY01`, CRC16,
automatic ACK and a fixed 32-byte packet. The packet contains sequence, GPS coordinates,
temperature, humidity, pressure, flags and survivor ID.

Receiver hotspot defaults are `RA-MAYDAY` / `Mayday2026`; open
`http://192.168.4.1/`. Change the example password before field use.

## Dashboard capabilities in the supplied source package

Eight survivor display slots, received GPS positions and trails, SOS event history,
stale-data detection, temperature/humidity/pressure trends, packet search and CSV export,
HQ/search-radius planning, checkpoints, GeoJSON/map-image export, team creation and
assignment, priority and mission-state tracking, operator notes/log, direct distance and
bearing, planning ETA, incident desk, optional audible SOS alert, receiver diagnostics,
settings, backup/restore, print and fullscreen controls.

The dashboard uses a local coordinate plot, not offline street tiles or obstacle-aware
routing. Team locations and assignments are operator-entered, browser-local and are not
transmitted to responder radios. ETA is a planning estimate, not a rescue prediction.
The 32-byte radio packet does not include breathing, battery, gas, altitude, satellite
count or indoor 3D position; no unsupported readings are fabricated.

## Build notes

After the editable dashboard HTML and embedded headers are committed, run:

```bash
python3 tools/build_dashboard.py
python3 tools/verify_package.py
```

These checks do not replace an Arduino board build, physical RF test, browser visual check
or field validation. This is an experimental prototype, not a certified life-safety system.
