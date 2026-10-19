# Proposal

## Why

The primary tinyGS environment is the classic ESP32, but `espressif/esp-emulator` does not
model that chip. The upstream Espressif QEMU fork does boot the ESP32, but its feature
matrix marks Wi-Fi as unsupported, so it cannot exercise tinyGS's AP/station, MQTT or web
flows. A community QEMU fork (`Ebiroll/qemu_esp32`, based on QEMU 9.2) adds an `esp32_wifi`
NIC model with slirp networking; if it works with the Arduino WiFi stack, the classic ESP32
can get the same hardware-free integration coverage that the S3/C3 already have.

## What Changes

- Add a **time-boxed spike** that builds a pinned commit of the ESP32 QEMU fork in Docker
  and runs the classic `ESP32` firmware with the `esp32_wifi` NIC model.
- If the spike passes, add the classic ESP32 as a target of the integration bench, reusing
  the same test structure (serial console over TCP, flash image, host port forwarding,
  test-server access) as the other emulator targets.
- Keep the upstream Espressif QEMU fork as a documented fallback for boot/serial-only
  checks, because it does not emulate Wi-Fi.
- Record the go/no-go result and the exact pinned fork commit so the decision is
  reproducible.
- Do not use Wokwi as the default path: it is a hosted service with a token and traffic
  limits, and it is not part of the self-hosted test infrastructure.

Out of scope: RF/LoRa, Bluetooth, display fidelity, production endpoints, and replacing
on-hardware validation. This change does not alter firmware behaviour.

## Capabilities

### New Capabilities

- `esp32-qemu-integration-tests`: a hardware-free integration path for the classic ESP32
  environment, covering boot, serial, WiFi/MQTT and the device web when the QEMU fork
  supports them, with an explicit fallback when it does not.

### Modified Capabilities

None. The unit-test suite and the S3/C3 emulator path are separate changes.

## Impact

- New test code and a Docker recipe/build step for the pinned QEMU fork; no firmware or
  release-environment changes.
- A test-only radio bypass is reused from the emulator bench so a configured boot does not
  block on the absent SX126x.
- Disk/time cost: the QEMU build is heavier than the prebuilt esp-emulator binary; the
  image and build cache are pinned and shared across runs.
- The classic ESP32 remains covered on hardware if the spike fails.
