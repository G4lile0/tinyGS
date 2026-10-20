# Design

## Context

The boot/serial QEMU tier works (`add-esp32-qemu-integration`, `tests/emu-esp32/SPIKE.md`).
The remaining blocker is the WiFi model: with
`-nic user,model=esp32_wifi`, the guest loops on `EM_RD NVDS 0x0000` after provisioning and
never gets an address. Project sources are frozen until a test-only change is explicitly
approved.

## Goals / Non-Goals

**Goals:**

- Determine the cause of the `EM_RD NVDS` loop and either fix it with a fixture/option or
  prove the model is not usable with the Arduino image.
- If usable, add WiFi, web and MQTT to the shared backend-neutral suite.
- Keep the test-only radio bypass conditional and out of release builds.

**Non-Goals:**

- RF/LoRa, Bluetooth, I2C/OLED or real-time behaviour.
- Production endpoints or accounts.
- Rewriting the shared tests for one backend.
- Changing firmware sources without a separate approval.

## Investigation plan

Run the following in order, recording the exact command and artifact for each:

1. **Baseline**: reproduce the `EM_RD NVDS` loop with the pinned fork and the merged
   Arduino image; capture the QEMU log and firmware console.
2. **IDF control**: boot an ESP-IDF WiFi example that the fork's author validated with the
   same `esp32_wifi` options. If it associates and DHCP works, the model is functional and
   the issue is in the Arduino/NVS/PHY path.
3. **NVS/PHY**: populate the `nvs` and `phy_init` partitions from a real ESP32 or from IDF
   `phy_init` data, then retry. Check whether the loop is a missing calibration blob.
4. **eFuse/PHY blob**: try the fork's `qemu_efuse.bin`, `make-efuse`/PHY options or a
   populated eFuse image. Check the fork's examples for the expected machine/NIC options.
5. **NIC options**: verify `-nic user,model=esp32_wifi` against `-netdev`/`-device`
   equivalents; check for a required channel/AP index (the fork has a hardcoded AP list:
   `Open Wifi`, `MasseyWifi`, ...).
6. **Compare with upstream**: confirm the upstream Espressif QEMU has no WiFi, so it is not
   a WiFi fallback.

If steps 1-5 make association work but the configured boot still blocks in radio init, then
and only then evaluate the test-only radio bypass.

## Decisions

- **Reuse the shared interface.** WiFi/MQTT/web tests stay in `tests/integration/tests/`;
  the backend provides credentials and network URLs. No backend branching in tests.
- **Claims follow tests.** The QEMU capability matrix gains `wifi`, `web` and `mqtt` only
  when the corresponding tests pass; until then the tests are skipped with a reason.
- **No source changes by default.** The radio bypass is a conditional last step and needs
  explicit approval. If it is added, it lives in a test-only PlatformIO environment and is
  absent from release builds.
- **No upstream fallback for WiFi.** The upstream QEMU fork does not emulate WiFi; keeping
  it as a WiFi fallback would be misleading. Boot/serial remains on the current fork.

## Risks / Trade-offs

- [The model may be incompatible with the Arduino stack] → Record the evidence and keep the
  tier boot/serial only; WiFi stays on hardware.
- [NVS/PHY fixtures may be board-specific] → Use a minimal, documented calibration blob and
  keep it under `tests/`; never ship it in firmware.
- [Test-only bypass could hide radio regressions] → Keep it out of release/HIL binaries and
  document it as an emulator-only flag.
- [Cloud tests can be flaky] → Dedicated station, cleanup in `finally`, retries only for
  infrastructure errors, and a three-run stability gate.
- [QEMU build is heavy] → Reuse the pinned image from the boot/serial change.

## Migration Plan

Additive to the existing test tree. If the investigation succeeds, enable the capabilities
and tests; if it fails, document the limitation and leave the boot/serial tier unchanged.
No firmware or release changes unless the bypass is approved.

## Open Questions

- Does the model need a populated `phy_init`/NVS partition, or does it synthesize
  calibration data itself?
- Does the Arduino core use a different NVS namespace/layout than the IDF examples the fork
  was validated against?
- Is the hardcoded AP list (`Open Wifi`, channel 4) usable for an open-network provisioning
  path, or does the model require WPA?
