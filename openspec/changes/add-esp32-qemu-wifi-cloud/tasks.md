# Tasks

## 1. Investigate the NVS/PHY blocker

- [ ] 1.1 Reproduce the `EM_RD NVDS 0x0000` loop with the pinned fork and the merged Arduino image; capture the QEMU log and firmware console under `tests/integration/artifacts/`.
- [ ] 1.2 Boot an ESP-IDF WiFi example validated by the fork author with the same `-nic user,model=esp32_wifi` options; verify whether it associates and obtains a lease, and record the result.
- [ ] 1.3 Populate the `nvs` and `phy_init` partitions from a real ESP32/IDF calibration blob and retry; verify whether the loop disappears and the firmware associates.
- [ ] 1.4 Try the fork's eFuse/PHY options, a populated eFuse image, and the `-netdev`/`-device` equivalent of `-nic`; verify which combination, if any, completes association.
- [ ] 1.5 Verify the hardcoded AP list and channel (`Open Wifi`, channel 4) against the firmware scan; verify the chosen SSID/password reaches association.
- [ ] 1.6 Record the findings in `tests/emu-esp32/SPIKE.md` (cause or ruled-out candidates); verify the document names the exact commands and artifacts.

## 2. Enable WiFi/web/MQTT if the investigation succeeds

- [ ] 2.1 Enable the shared provisioning test on `esp32-qemu` and verify association plus an IPv4 address.
- [ ] 2.2 Enable the shared `hostfwd` web test and verify the tinyGS page is served on the host port.
- [ ] 2.3 Enable the outbound TCP/MQTT test with the test credentials and verify the test broker exchange.
- [ ] 2.4 Add the capability claims `wifi`, `web` and `mqtt` to the QEMU backend only after 2.1-2.3 pass; verify the shared tests run instead of skipping.
- [ ] 2.5 Add the cloud opt-in, production-host guard and station cleanup in `finally`; verify missing credentials skip cloud tests and production hosts are rejected.
- [ ] 2.6 Add the auto-OTA watchdog; verify it fails on a synthetic update line and stays silent on a normal boot.

## 3. Conditional test-only radio bypass

- [ ] 3.1 Confirm that the configured QEMU boot reaches WiFi but still blocks in radio init; record the exact console point.
- [ ] 3.2 Ask for approval before touching firmware sources; if approved, add `TINYGS_EMU_NO_RADIO` to a test-only environment and guard radio init.
- [ ] 3.3 Verify the bypass is absent from release environments and from the boot/serial QEMU image; verify the configured test image reaches MQTT.

## 4. Stability and documentation

- [ ] 4.1 Run the full WiFi/web/MQTT path three times from fresh flash copies; verify 3/3 pass and collect artifacts.
- [ ] 4.2 Update `tests/README.md`, `tests/integration/README.md` and `tests/emu-esp32/SPIKE.md` with the final capability matrix and limitations.
- [ ] 4.3 Verify the same test files run on `esp32s3-emu` and `esp32-qemu` without backend-specific branches.

## Workflow follow-up

- If the investigation fails, keep the QEMU tier boot/serial only and leave WiFi/MQTT/web on hardware.
- Archive this change only after the capability matrix and docs match the observed behavior.
