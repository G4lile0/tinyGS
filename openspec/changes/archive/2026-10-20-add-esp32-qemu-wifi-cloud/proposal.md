# Proposal

## Why

The classic ESP32 QEMU tier already covers boot, serial and reset. This change investigated
whether the pinned fork's `esp32_wifi` model could add WiFi, the device web and MQTT to that
tier. The answer is no with the current model:

- the Arduino WiFi stack used by tinyGS does not complete association (`status=0`);
- an ESP-IDF 5.4 control associates but never receives a DHCP lease;
- the `EM_RD NVDS` log is Bluetooth exchange-memory debug noise, not the WiFi failure;
- NVS/PHY is not the blocker: the IDF control falls back to full calibration and still
  associates;
- `-device esp32_wifi` is not supported; only `-nic user,model=esp32_wifi` works.

## What Changes

None in code or specs. This change records the negative result and closes the
investigation. The `esp32-qemu` backend stays **boot/serial/reset only**; WiFi/MQTT/web
stay on hardware. No tinyGS sources, release environments or test capabilities change.

The evidence and reproduction commands are in `tests/emu-esp32/SPIKE.md` and
`tests/integration/artifacts/`.

## Capabilities

None. This is a documentation/decision record, so the change sets `skip_specs: true`. The
existing `esp32-qemu-integration-tests` capability already describes the boot/serial tier.

## Impact

- Documentation only: `tests/emu-esp32/SPIKE.md` and the test READMEs.
- No QEMU fork patch, no firmware flag, no new dependency.
- Future work (fixing the model) is listed in the tasks as plain follow-up bullets, not as
  committed behavior.
