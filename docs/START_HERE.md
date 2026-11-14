# RA MAYDAY — Tactical Receiver v3.0

This replaces the firmware on your **classic ESP32 receiver**, not the ESP32-S3 sender.
No AWS, Pi, internet, Python, CloudShell, router, or filesystem uploader is needed.

## 1. Wiring — receiver only

Keep power off while checking connections. These pins avoid GPIO 4, 5, 18 and 34.

| nRF24 pin | Classic ESP32 GPIO |
|---|---|
| CE | 25 |
| CSN | 26 |
| SCK | 27 |
| MISO | 19 |
| MOSI | 23 |
| VCC | Stable 3.3 V |
| GND | GND |

**PA+LNA power is critical:** do not assume the ESP32 3.3 V pin can supply the radio's transmit-current peaks. Use a clean, regulated 3.3 V supply sized for the module's peak current (follow the module vendor's rating), connect grounds together, and place a 10–47 µF low-ESR capacitor plus a 100 nF ceramic capacitor close to the radio VCC/GND pins. Keep SPI wires short. Never connect nRF24 VCC to 5 V.

The receiver ACK transmitter is configured at RF24_PA_HIGH to improve the return link. For a very-close bench test, RF24_PA_LOW may reduce overload. The sender currently uses RF24_PA_MAX; only retain that setting with a stable supply and appropriate antenna/module installation. Do not assume advertised range: measure reliable packet delivery in the actual environment.
No receiver-to-Pi UART wires are needed. Receiver sensor readings come from the sender.

## 2. Open and upload

1. Extract this ZIP.
2. Open `RA_MAYDAY_Offline/RA_MAYDAY_Offline.ino` inside the extracted folder.
   Arduino IDE must also show `Dashboard.h` and `CommandBackground.h` tabs. Keep them together.
3. Boards Manager: install **esp32 by Espressif Systems** if not already installed.
4. Library Manager: install **RF24 by TMRh20**.
5. Choose **Tools → Board → ESP32 Arduino → ESP32 Dev Module** for the classic receiver.
6. Choose **Tools → Port** for that receiver. Close other serial terminals.
7. Click Upload. If it stays at Connecting, hold BOOT until upload begins, then release.
8. Open Serial Monitor at **115200 baud**. If necessary press EN/RESET once.
9. Look for `[WEB] http://192.168.4.1` and `[NRF24] READY`.

WiFi, WebServer and SPI come with the Espressif board package. No ArduinoJson is needed.
Website HTML, JavaScript, CSS and vector background are compiled into flash. No separate
web hosting or filesystem upload is needed. Only upload the receiver sketch.

## 3. Connect your laptop/phone

- Wi-Fi name: **RA-MAYDAY**
- Wi-Fi password: **Mayday2026** (case-sensitive)
- Browser address: **http://192.168.4.1/** — explicitly use HTTP, not HTTPS.

Stay connected when the phone reports "No internet". Disable automatic switching to
another Wi-Fi network if it disconnects. Connect to RA-MAYDAY, not the sender's hotspot.
Any joined client can read telemetry. Change AP_PASSWORD in the sketch before field use.
The AP allows up to four connected devices; start with one dashboard browser.

## 4. Verify real telemetry

Power the existing ESP32-S3 sender. The receiver keeps the sender's protocol:
32-byte fixed payload, MAY01, channel 76, 250 kbps, CRC16, automatic acknowledgement.
Sequence increases and the survivor's ID appears in the registry. Hold SOS for two
seconds, release, and check both timeline transitions. Turn off the sender and verify
stale status after the configured threshold (20 seconds by default). Cached SOS remains visible; it is not silently cleared.
GPS NO FIX is expected until the sender produces valid coordinates. No positions are
invented. The local page runs normally even when there are no packets or GPS fix.

## 5. Team assignment and planning ETA

1. Click Add team, give it a name, members and realistic travel speed.
2. Enter both team coordinates if known; leave both blank otherwise.
3. Choose the received survivor ID and team, priority, preparation time and notes.
4. Save assignment. Confirm dispatch through voice/radio yourself.
5. Update Assigned → En route → On scene → Rescued manually as work progresses.

The website stores teams, missions and a 50-entry operator log in this browser's
localStorage. Other browsers do not share those edits. Export operations JSON for a
record/backup. Use Restore backup to replace this browser’s operations with a previously exported JSON. Imports are validated; there is no merge. SOS acknowledgements are not restored. Private browsing/storage clearing can
remove records. If the browser blocks storage, keep the tab open and export.
Assignments are NOT transmitted to sender radios or responder devices. Multiple
operators should designate one browser as the assignment authority.

ETA = straight-line distance / entered speed + entered preparation time, rounded up.
It is a planning estimate, NOT a road route, safe path, extraction-time prediction, or
survival estimate. It requires a fresh survivor GPS fix, entered team coordinates and
a positive speed. Team positions remain manual; the entry timestamp is shown.
ETA is hidden for stale/no-fix/offline data and completed/cancelled missions.

## 6. Included features and limits

- Graphite-and-amber vector background, custom rescue logo, responsive dark interface, section navigation.
- Eight distinct survivor IDs, last-report age, SOS, live/stale status, sensor health.
- Animated geographic GPS plot with team positions and straight assignment lines.
  This is not a street map: no offline road tiles are bundled. OpenStreetMap links
  require internet. No marker motion is inferred from the decorative pulse animation.
- Selected survivor temperature/humidity/pressure trends and coordinates.
- 120 recent packets across all IDs, searchable packet table and CSV export.
- Last 32 SOS transitions stored on the receiver independently of the packet history.
- Team editor, priorities, notes, mission status and operator activity log.
- Receiver radio readiness, client count, free memory, uptime and capacity counters.
- All receiver packet/event history is RAM-only and resets on reboot.
- Receiver uptime timestamps avoid pretending the offline board has UTC clock sync.
- CSI, MQ-135, battery and GPS satellite count are absent from the 32-byte sender
  packet and therefore are not invented or displayed. CSI stays on the sender.
- Multiple senders need distinct IDs and matching radio settings. Eight display slots
  do not guarantee radio capacity or collision-free reception. Extra IDs are counted
  as capacity drops until receiver reboot. Test one sender first.
- The radio worker runs separately from HTTP handling to keep reception responsive.
  This does not guarantee lossless delivery under load or interference.

## 7. If something does not work

- RF24.h missing: install RF24 by TMRh20 in Library Manager.
- Dashboard.h/CommandBackground.h missing: extract the ZIP fully, keep all three sketch files together.
- NRF24 FAILED: check pin map, ground, stable supply, seating and radio orientation.
- Page loads but no ID: ensure the sender is running and radio configuration matches.
- No GPS position: wait for a real sender fix with the antenna's appropriate sky view.
- No hotspot: inspect Serial Monitor and verify you flashed the RECEIVER board.
- Wrong page: forget the sender hotspot and reconnect specifically to RA-MAYDAY.
- Upload port busy: close all other Serial Monitor/miniterm/screen instances.

## Verification performed

JavaScript syntax and simulated-DOM behavior checks passed for empty/live/stale/SOS,
team creation, assignment, persistence writes, busy-team protection and ETA suppression.
Host C++ tests passed for packet layout/offsets, SOS edges, buffer wrap, ID validation
and eight-ID capacity. These are not an ESP32 compilation or physical radio test.
The environment could not launch a browser for visual checks. A real Arduino build,
mobile appearance check and on-device end-to-end validation remain necessary.

This is an operational prototype, not a certified life-safety system.

## Receiver console v2 additions

The receiver now has its own graphite/amber instrument-console identity, a lightweight
vector command-network background, full-screen control and print-friendly reports.
No external fonts, image services or animation libraries are required.

- Incident desk: operator-assigned priority, contact status, reported person count,
  situation notes and SOS acknowledgement. These are manual, not medical triage.
- Optional SOS sound: enable it with a click. One sound per new, fresh, unacknowledged
  episode while this tab is active; browsers can suspend audio. Never rely on sound alone.
- Map zoom in/out, fit view and reduced-motion preference.
- Preferences: station label, 5–300 second stale threshold, 2/5/10-second refresh,
  1–10 travel multiplier and decorative motion. All stored in the viewing browser.
- Planning ETA applies the multiplier only to travel time, then adds preparation.
- Diagnostics: measured HTTP round trip, radio state, buffer use and storage availability.
- JSON backup restore: replaces local operational records after an in-page confirmation;
  validates types/ranges and team references. Current SOS must be acknowledged again.
- Manual pre-deployment checklist, retained in this browser.
- Observed receiver restarts clear operator SOS acknowledgements but retain assignments.

The extended simulated-DOM checks cover settings, incident entry, acknowledgement,
backup validation, unsafe input rendered as text, and acknowledgement reset on reboot.
ESP32 build, physical hardware and full visual rendering remain unverified here.

## Tactical map v3 — reference-inspired command layout

The principal view now places field intelligence, a large tactical GPS map, and
selected-survivor telemetry beside one another on a wide display. On phones, the map
comes first and the panels stack. The vector background is unique to the receiver.

### Map controls

- **Set HQ & search radius**: enter the base coordinates and a 10–50,000 metre radius.
  The circle is a manual search boundary, NOT radio coverage or detection range.
- **Drag** the map to pan; scroll or use +/− to zoom. **Fit view** resets the view.
- **Follow target** centres the selected survivor as received fixes change.
- **Click a survivor marker** or use the selector to choose the target.
- **Grid / GPS trails / Sweep** toggle their layers. The sweep is decorative.
- **Add checkpoint** then click the map to place a named point. **Remove last point**
  undoes the most recent checkpoint. Up to 30 operator-entered checkpoints are retained.
- **Export GeoJSON** saves coordinates with received/operator-entered source labels.
- **Save map image** saves the current canvas as a PNG.

The target card displays direct distance and initial true bearing from the assigned
team's entered position, or from entered HQ if no positioned team is assigned.
Received trails use the last 120 shared packets and break at missing GPS / long gaps.
Last-known GPS positions remain visible but are explicitly marked STALE if a fresh
valid fix is absent or the receiver disconnects. The planning ETA still requires a
fresh valid current report.

The coordinate map uses a local north-up approximation suited to a rescue sector;
it is not a surveyed map, street map, obstacle-aware route or global navigation tool.
It has no fabricated buildings, roads, terrain, radio strengths or survivor vitals.
All operations, HQ and checkpoints are stored in this browser and included in validated
JSON backups. Other clients have independent assignment and map settings.

v3 automated checks additionally cover projection, cardinal bearing, HQ/search radius,
checkpoint add/undo, pan/zoom/fit/follow, layer toggles, last-known-fix handling and backup
validation. All UI checks use a simulated DOM; actual browser appearance, ESP32 build
and physical radio operation have not been verified in this environment.


## Sender and receiver appearance

The current sender sketch has been updated to use the same graphite-and-amber command-console visual theme as the receiver dashboard. The sender-local portal and receiver command dashboard remain separate pages with different functions.
