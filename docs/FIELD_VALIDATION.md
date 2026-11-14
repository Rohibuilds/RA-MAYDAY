# RA MAYDAY field validation and release gate

This checklist separates source-level checks from tests that require the actual ESP32-S3,
classic ESP32, nRF24L01+ modules, sensors, GPS antenna and a field operator. A green
GitHub Actions run is **not** evidence that the radio link or rescue workflow is safe.

## 1. Before powering hardware

- [ ] Confirm the sender board is the intended ESP32-S3 variant and the receiver is a
      classic ESP32; do not flash the receiver sketch to the sender.
- [ ] Check every pin against the wiring table in START_HERE.md.
- [ ] Check common ground and stable 3.3 V supply for each nRF24L01+.
- [ ] Verify decoupling at the radio module; use a suitable regulator for PA/LNA modules.
- [ ] Inspect polarity, exposed conductors, antenna connections and enclosure strain relief.
- [ ] Replace the example Wi-Fi AP credentials in the receiver sketch before deployment.
- [ ] Assign unique survivor IDs; document the ID-to-person/device mapping securely.
- [ ] Confirm the radio channel, address, data rate, CRC and 32-byte packet layout match.

## 2. Source and build validation

- [ ] GitHub Actions source-validation workflow passes on the exact release commit.
- [ ] Run `python3 tools/verify_package.py` locally.
- [ ] Compile sender firmware for the exact ESP32-S3 board package and record its version.
- [ ] Compile receiver firmware for the exact classic ESP32 board package and record its version.
- [ ] Record Arduino-ESP32 core and every installed library version.
- [ ] Confirm no build warnings were silently ignored; review memory usage and partition limits.
- [ ] Confirm Dashboard.h and CommandBackground.h are regenerated after web asset edits.

The automated workflow checks package integrity, dashboard JavaScript syntax, and compiles
both configured Arduino targets (ESP32-S3 sender and classic ESP32 receiver) using the
workflow's configured ESP32 core and installed libraries. CI compilation does not prove that
the exact physical boards, wiring, radio link, sensors, GPS or field workflow operate correctly.

## 3. Bench test: radio and telemetry

Run with one sender and one receiver first. Keep both devices stationary and close together.

- [ ] Receiver hotspot starts and the dashboard loads at http://192.168.4.1/.
- [ ] Receiver reports radio ready; sender reports successful radio initialization.
- [ ] Correct survivor ID appears and packet sequence advances.
- [ ] GPS-invalid state is shown until a real valid fix is received.
- [ ] Latitude/longitude match an independently checked location within expected GPS accuracy.
- [ ] Temperature, humidity and pressure are plausible and update when conditions change.
- [ ] Sensor-fault state is visible when a sensor is disconnected (test only on bench).
- [ ] SOS activation and release each create the correct transition exactly once.
- [ ] Stopping the sender causes stale/offline status after the configured threshold.
- [ ] Cached SOS remains visible after sender loss; it is never silently interpreted as cleared.
- [ ] Receiver reboot behavior is understood: radio/event RAM history resets.
- [ ] Test at least 30 minutes of continuous packets and inspect rejected/capacity counters.

## 4. Range, interference and loss testing

- [ ] Test increasing distances in the actual intended environment.
- [ ] Repeat with realistic body obstruction, antenna orientation and enclosure installed.
- [ ] Record packet counts sent and received, gaps, reconnects and RSSI if available.
- [ ] Test nearby Wi-Fi activity and other 2.4 GHz interference.
- [ ] Verify behavior after sender power loss, receiver power loss and brownout.
- [ ] Test each additional sender one at a time, then test the intended simultaneous count.
- [ ] Do not assume eight dashboard slots mean eight collision-free radio transmitters.
- [ ] Establish a tested operating range with a conservative margin; do not infer range from bench success.

## 5. Dashboard and operations

- [ ] Test on the actual operator laptop and at least one target phone/browser.
- [ ] Verify map pan, zoom, fit, target selection, GPS trails and checkpoint placement.
- [ ] Verify HQ and search radius are clearly treated as operator planning aids.
- [ ] Verify team assignments, notes and mission states survive refresh in the same browser.
- [ ] Export a backup, restore it in a test browser and verify expected records.
- [ ] Confirm a second browser does not silently appear synchronized; assignments are local.
- [ ] Verify ETA disappears for stale/no-fix data and is labelled as straight-line planning only.
- [ ] Test CSV, GeoJSON, map image, print and fullscreen controls.
- [ ] Test SOS acknowledgement, optional sound and browser-tab suspension.
- [ ] Confirm operator actions are communicated over a separate, tested field channel.

## 6. Field-readiness gate

Do not use the prototype as the sole emergency communication or rescue system. Before any
real exercise, appoint a human incident lead, define radio check-in intervals, backup
communications, lost-link procedures, battery reserve, evacuation criteria and manual
location verification. Conduct a supervised drill with dummy participants first.

A release is **not field-approved** until all hardware, range, browser, power, weatherproofing,
human-factors and operational checks above have named owners, recorded results and sign-off.
This project is an experimental prototype and is not a certified life-safety device.

## Test record template

| Field | Record |
|---|---|
| Commit / release | |
| Sender board + firmware build | |
| Receiver board + firmware build | |
| Arduino-ESP32 core version | |
| RF24 and sensor library versions | |
| Radio modules / antennas / supply | |
| Test location and environment | |
| Number of senders / duration | |
| Packets sent / received / gaps | |
| Tested reliable range + margin | |
| Browser/device matrix | |
| Failures and corrective actions | |
| Operator / reviewer / date | |
