# Tasks

## 1. Investigation

- [x] 1.1 Reproduce the WiFi failure with the pinned fork and the merged Arduino image; capture the firmware console and QEMU log under `tests/integration/artifacts/wifi-repro/`.
- [x] 1.2 Build and run an ESP-IDF 5.4 station control with `-nic user,model=esp32_wifi`; verify that it associates but does not obtain a DHCP lease.
- [x] 1.3 Rule out NVS/PHY as the blocker: the IDF control logs a full-calibration fallback and still associates; no populated calibration blob is required to reproduce the failure.
- [x] 1.4 Check the `-netdev`/`-device` form and the eFuse/PHY options: `-device esp32_wifi` is not a pluggable type; `-nic user,model=esp32_wifi` is the only working instantiation.
- [x] 1.5 Verify the hardcoded AP list and channel: the Arduino scan sees all six APs, including `Open Wifi` on channel 4 with open security.
- [x] 1.6 Record the corrected diagnosis, commands and artifact paths in `tests/emu-esp32/SPIKE.md`.

## 2. Decision

- [x] 2.1 Close the change as a negative result: the model is not usable for tinyGS WiFi/MQTT/web.
- [x] 2.2 Keep the `esp32-qemu` capability matrix at `serial`, `reset`, `flash`, `improv`; leave `test_wifi.py` skipped with an explicit reason.
- [x] 2.3 Confirm no project source changes, no QEMU fork patch and no new dependency were added.

## Future work (not part of this closed change)

- Fix the QEMU `esp32_wifi` model: accept `STATE_STA_DHCP` in `Esp32_WLAN_can_receive()`, verify the DHCP offer parsing, then debug Arduino association (capability info/IE set).
- Re-run the Arduino scan/connect and IDF station controls, then the tinyGS provisioning/web/MQTT tests.
- Only after WiFi works, evaluate a test-only `TINYGS_EMU_NO_RADIO` flag to reach MQTT.
- Open a new OpenSpec change when there is a concrete model fix to implement.
