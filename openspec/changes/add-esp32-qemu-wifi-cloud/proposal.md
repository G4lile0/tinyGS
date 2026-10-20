# Proposal

## Why

The classic ESP32 QEMU tier boots and exposes a working serial/reset path, but its WiFi
model does not complete initialisation with the Arduino image: after Improv provisioning
the guest loops on `EM_RD NVDS 0x0000`, so there is no IPv4 address, MQTT, or device web.
Without WiFi, the classic ESP32 cannot join the same integration coverage that the
esp-emulator targets have.

## What Changes

- Investigate why the pinned QEMU fork's `esp32_wifi` model loops on `EM_RD NVDS` with the
  Arduino image. Candidates: missing PHY/NVS calibration data, a required eFuse/PHY blob,
  a machine/NIC option, or an NVS layout the model does not handle.
- If a populated NVS/PHY image or a QEMU option fixes it, add the provisioning path and the
  WiFi/web/MQTT tests to the shared backend-neutral suite.
- If a configured boot still blocks in radio init, evaluate a **test-only** radio bypass.
  That is a firmware change and requires explicit approval; project sources stay frozen
  until then.
- Enable `wifi`, `web` and `mqtt` capability claims only after the corresponding shared
  tests pass on the QEMU backend.
- Add the cloud opt-in, production-host guard, test-station cleanup and auto-OTA watchdog
  from the other integration tier.
- Run the WiFi/cloud path three times from fresh flash copies and record stability.

Out of scope: RF/LoRa, Bluetooth, I2C/OLED, real-time behaviour, production endpoints and
any release-binary change.

## Capabilities

### New Capabilities

- `esp32-qemu-wifi-cloud`: WiFi, device web, MQTT and opt-in test-server coverage for the
  classic ESP32 QEMU backend, with a documented NVS/PHY investigation and a test-only
  radio bypass if needed.

### Modified Capabilities

- `esp32-qemu-integration-tests`: its QEMU capability matrix gains `wifi`, `web` and
  `mqtt` once the new tests pass.

## Impact

- Test code, QEMU/NVS fixtures and possibly a test-only firmware flag; no release change.
- The shared suite and target interface are reused; test files stay backend-neutral.
- Cloud runs need a dedicated test station and are opt-in.
- If the investigation fails, the QEMU tier remains boot/serial only and the WiFi/MQTT/web
  coverage stays on hardware.
