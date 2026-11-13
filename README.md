# RA MAYDAY · COLOSSUS 3.0

Offline field telemetry and rescue coordination built around an ESP32-S3 sender,
nRF24L01+ radio link, and a classic ESP32 receiver hosting its own tactical dashboard.

**Status:** working source package with host-side logic tests. The current complete
firmware has not been compiled or tested on physical boards in this environment.
Earlier sender logs reported successful radio transmissions; they do not validate
all subsequent firmware changes. This is an experimental prototype.

## Start here

1. Install **esp32 by Espressif Systems** in Arduino IDE Boards Manager.
2. Install the libraries listed in [dependencies](docs/dependencies.md).
3. Upload `firmware/RA_MAYDAY_Sender/RA_MAYDAY_Sender.ino` to the **ESP32-S3 sender**,
   or retain your matching existing sender firmware.
4. Upload `firmware/RA_MAYDAY_Offline/RA_MAYDAY_Offline.ino` to the **classic ESP32 receiver**.
   Keep its `.h` files in the same sketch folder.
5. Join receiver Wi-Fi **RA-MAYDAY**, example password **Mayday2026**.
6. Open **http://192.168.4.1/**. No internet or Raspberry Pi is required.

Change example hotspot passwords before sharing the hardware. The sender's separate
hotspot is **RA-MAYDAY-SURVIVOR**, example password **MAYDAY2026**.

See [wiring](docs/wiring.md) and [full receiver guide](docs/receiver-setup.md).

## Features

- Sender GPS, AHT20/BMP280 telemetry, SOS button, OLED and WS2812 status.
- Sender-local experimental Wi-Fi CSI view (not transported in the radio packet).
- Receiver tactical map, real received GPS trails, search area, HQ and checkpoints.
- Eight survivor display slots, selected telemetry, SOS history and stale reporting.
- Manual team assignments, priorities, incident notes and mission progress.
- Travel planning estimates from entered team positions/speeds, not rescue predictions.
- Browser-local settings and operational backup/restore, CSV/GeoJSON/map export.
- Green/red receiver status LEDs on GPIO 32/33.

## Repository layout

| Directory | Purpose |
|---|---|
| `firmware/RA_MAYDAY_Sender` | ESP32-S3 field sender, with embedded sender interface |
| `firmware/RA_MAYDAY_Offline` | Classic ESP32 receiver, embedded tactical dashboard, LEDs |
| `web/receiver` | Editable receiver HTML and vector background |
| `tools/embed_dashboard.py` | Rebuild receiver headers from website source |
| `tests` | Host-side UI, packet-buffer and LED logic checks |
| `docs` | Wiring, setup, dependencies and limitations |
| `optional/aws` | Earlier optional Lambda/DynamoDB cloud variant; not required offline |

After editing the receiver website, run `python3 tools/embed_dashboard.py` before
uploading the receiver sketch. GitHub stores source: GitHub Pages alone cannot receive
these radio packets. The operational offline site is served by the receiver ESP32.

## Local verification

From this directory (Python 3, Node.js 18+ and a C++17 compiler):

```bash
python3 tools/embed_dashboard.py --check
node tests/dashboard.cjs
python3 tests/packet_led.py
python3 tests/cloud_api.py
```

These tests use a simulated DOM, mocked cloud storage and host C++ logic. They do not
replace an Arduino board build, RF link test, browser visual check or field validation.

## Data limits

Radio payload: 32 bytes, pipe `MAY01`, channel 76, 250 kbps, CRC16, automatic ACK.
Temperature/humidity/pressure, sequence, ID, GPS coordinates and three flags are sent.
Battery, gas, breathing, indoor 3D position, altitude and satellite count are not in
this packet. No unsupported readings are fabricated.

Team positions/assignments are operator-entered, browser-local and not transmitted to
responder devices. Receiver telemetry/event history is in RAM and resets on reboot.
Map search circles are planning boundaries, not RF detection range. No street tiles,
obstacle routing or certified life-safety capability is included.

## Cloud variant

The optional AWS files are an alternative deployment, not part of the offline setup.
Copy `CloudConfig.example.h` to `CloudConfig.h` beside that optional receiver sketch,
then configure it locally. The real header, tokens and deployment state are ignored.
AWS resources can incur charges. No AWS deployment has been verified in this project.

## Attribution and distribution

Firmware uses Arduino/ESP32 and third-party libraries under their respective licenses.
The source package has no project-wide open-source license selected yet; publishing
it does not automatically grant reuse rights. Image assets were generated for this
project; the receiver background is an original SVG included in the source tree.
