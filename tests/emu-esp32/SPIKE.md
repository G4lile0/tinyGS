# Spike result: classic ESP32 QEMU

Date: 2026-10-20
Host: Linux x86_64, Docker 29.9.1
Fork: `Ebiroll/calib` at `73bee9bab5a78b0aaa584610b1564d533e7879ce` (QEMU 9.2.2)
Image: `tinygs-qemu-esp32` (240 MB, first build 350 s, cached rebuild 4 s)
Firmware: merged 4 MB `ESP32` image (`tests/emu-esp32/make_image.py`)

## Checklist

| Check | Result | Evidence |
|---|---|---|
| Pinned fork builds in Docker | PASS | `qemu-system-xtensa --version` = 9.2.2; `-machine help` lists `esp32` |
| Classic ESP32 boots to TinyGS banner and controls | PASS | shared `test_boot_reaches_tinygs_console` |
| UART over TCP and console command | PASS | shared `test_console_command_is_answered` |
| QEMU monitor `system_reset` and serial reconnect | PASS | shared `test_reset_boots_the_same_firmware_again` |
| QEMU monitor `quit` stops the process | PASS | backend `stop()` waits for the container to exit |
| Improv provisioning over serial | FAIL | firmware prints `Wifi status: 0` and `Failed to connect to wifi...` |
| WiFi scan sees the emulated APs | PASS | Arduino control scan lists all 6 APs, including `Open Wifi` (ch 4, open) |
| Arduino `WiFi.begin("Open Wifi", "", 4)` | FAIL | status 0 (no association) |
| ESP-IDF 5.4 station example | PARTIAL | associates (`wifi:connected with Open Wifi, channel 4`) but no `GOT_IP` |
| `-netdev`/`-device` alternative | FAIL | `-device esp32_wifi,netdev=n0`: "Parameter 'driver' expects a pluggable device type" |
| `hostfwd` to the device web | FAIL | `http://127.0.0.1:<port>/` timed out |

## Diagnosis

The `EM_RD NVDS 0x0000` lines are **Bluetooth exchange-memory debug logging** from
`hw/xtensa/esp32.c` (`btdm_em_debug_level = 1`), not the WiFi failure. The real firmware
console after provisioning is:

```
IMPROV... Wifi status: 0
Failed to connect to wifi...
```

The model exposes the right APs: an Arduino scan control sees all six hardcoded APs,
including `Open Wifi` on channel 4 with open security. Association is the problem:

- Arduino (`WiFi.begin("Open Wifi", "", 4)`) never associates: status stays 0.
- ESP-IDF 5.4 associates at the WiFi level:
  `wifi:connected with Open Wifi, aid = 1, channel 4, bssid = 10:01:00:c4:0a:51`,
  but never receives `IP_EVENT_STA_GOT_IP`; the test prints `RESULT FAILED` after 60 s.
- The IDF log shows `phy_init: failed to load RF calibration data ... falling back to
  full calibration` and still associates, so NVS/PHY calibration is not the blocker.
- The model is not user-creatable via `-device`; only `-nic user,model=esp32_wifi`
  instantiates it.

Conclusion: the community `esp32_wifi` model is not usable for tinyGS yet. The Arduino
WiFi stack does not complete association, and even the IDF path that associates does not
get a DHCP lease through the model.

## Decision

- Keep the QEMU fork as a **boot/serial/reset** backend for the classic ESP32.
- Do **not** claim WiFi, MQTT or device-web coverage for `esp32-qemu`.
- Do **not** add a test-only radio bypass or any other firmware change: project sources are
  frozen for now.
- Keep WiFi/MQTT/web coverage on hardware. The upstream Espressif QEMU fork does not
  emulate WiFi, so it is not a replacement.

## Artifacts

- `tests/integration/artifacts/wifi-repro/` — tinyGS console + QEMU log + summary
  (`Wifi status: 0`, `Failed to connect to wifi...`).
- `tests/integration/artifacts/wifi-scan/` — Arduino scan control (6 APs).
- `tests/integration/artifacts/wifi-connect2/` — Arduino connect with pinned channel
  (status 0).
- `tests/integration/artifacts/idf-wifi/` — ESP-IDF 5.4 control (association, no GOT_IP).

## Next steps (needs approval)

1. Fix or replace the `esp32_wifi` model: association for the Arduino WiFi stack and a
   working DHCP path. This is a change to the test dependency (QEMU fork), not to tinyGS.
2. If a configured boot then still blocks in radio init, evaluate a test-only radio bypass.
   That requires a firmware change and is intentionally out of scope today.
3. Only then add `web`/`mqtt` capability claims and the corresponding shared tests.
4. Alternative hosted option: Wokwi, which emulates WiFi for the classic ESP32, at the cost
   of a token and a hosted dependency.
