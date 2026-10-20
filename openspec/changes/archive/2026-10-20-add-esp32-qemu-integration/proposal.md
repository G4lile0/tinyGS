# Proposal

## Why

The primary tinyGS environment is the classic ESP32, but `espressif/esp-emulator` does not
model that chip. The upstream Espressif QEMU fork boots the ESP32 but does not emulate
WiFi; a community fork (`Ebiroll/calib`, QEMU 9.2) adds an `esp32_wifi` NIC model. The
fork builds and boots the tinyGS classic ESP32 image, so it can provide a hardware-free
**boot, serial and reset** tier for that environment. WiFi/MQTT/web need more work and are
tracked separately.

## What Changes

- Add a Docker recipe under `tests/emu-esp32/` that pins and builds the QEMU fork with
  `xtensa-softmmu`, SLIRP and no GTK/SDL.
- Add a **backend-neutral** integration suite in `tests/integration/` where the same test
  files run against esp-emulator (ESP32-S3/C3) and the classic ESP32 QEMU backend; `--target`
  selects the backend.
- Cover boot, serial console, reset and the QEMU monitor for the classic ESP32.
- Keep the esp-emulator backend as the second implementation of the same interface, so the
  shared tests are real and not QEMU-specific.
- Document the QEMU build, the pinned commit, the capability matrix and the known
  WiFi/NVS limitation.

Out of scope for this change: WiFi association, MQTT, the device web, test-server/cloud
runs, the test-only radio bypass and the `EM_RD NVDS` investigation. They are tracked in
`add-esp32-qemu-wifi-cloud`.

## Capabilities

### New Capabilities

- `esp32-qemu-integration-tests`: a hardware-free boot/serial/reset path for the classic
  ESP32 environment, behind the same target interface as the other emulator backends.

### Modified Capabilities

None. The change does not alter firmware behaviour or existing specs.

## Impact

- New test code, a Docker recipe and a pinned QEMU build; no firmware or release-environment
  changes.
- The shared target interface keeps the tests backend-neutral, so a future WiFi/MQTT
  backend can be added without rewriting the test files.
- Disk/time cost: the QEMU build is heavier than the prebuilt esp-emulator binary; the
  image and build cache are pinned and shared across runs.
- Classic ESP32 WiFi/MQTT/web stay on hardware until the follow-up change.
