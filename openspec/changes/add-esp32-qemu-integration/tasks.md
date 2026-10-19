# Tasks

## 1. Pin and build the fork

- [ ] 1.1 Add a Docker recipe under `tests/emu-esp32/` that clones the QEMU fork at a pinned commit and builds `xtensa-softmmu` with SLIRP and without GTK/SDL; verify `qemu-system-xtensa --version` and `-machine esp32` are available in the image.
- [ ] 1.2 Document the pinned commit, build options, image size and first-build duration; verify a second build from the cache is significantly faster.

## 2. Boot and serial spike

- [ ] 2.1 Build the `ESP32` merged 4 MB image (or use the release `firmware.full.bin`) and boot it with `-machine esp32`, `-nographic` and the MTD flash drive; verify the console reaches the TinyGS banner and control menu.
- [ ] 2.2 Expose UART with `-serial tcp::<port>,server,nowait` and connect with `pyserial`; verify a console command and its response, and that the stream survives a QEMU monitor `system_reset`.
- [ ] 2.3 Expose the QEMU monitor on a TCP port and verify `system_reset` and `quit` terminate or restart the run as expected.

## 3. WiFi spike

- [ ] 3.1 Start QEMU with `-nic user,model=esp32_wifi` and provision the WiFi credentials over Improv; verify the firmware acknowledges the credentials and reboots.
- [ ] 3.2 Verify the firmware associates with the emulated network and obtains an IPv4 address; record the address and the console evidence.
- [ ] 3.3 Verify an outbound TCP connection from the firmware to a host listener (or to the test MQTT broker) succeeds through the NIC model.
- [ ] 3.4 Verify `hostfwd=tcp::<host-port>-:80` reaches the tinyGS configuration page; if the model does not support it, record the limitation and skip the web check.
- [ ] 3.5 Repeat the WiFi path three times from a fresh flash copy; verify the result is stable.

## 4. Radio and test-server integration

- [ ] 4.1 Build the `ESP32` test image with `TINYGS_EMU_NO_RADIO=1` and verify a configured boot reaches WiFi/MQTT logic instead of blocking in radio initialisation.
- [ ] 4.2 Add the cloud opt-in, production-host guard and test-station cleanup rules to the ESP32 QEMU tier; verify missing credentials skip the cloud cases and production hosts are rejected.
- [ ] 4.3 Add the auto-OTA watchdog and verify it fails on a synthetic update line and stays silent on a normal boot.

## 5. Go/no-go and fallback

- [ ] 5.1 Run the full spike three times and record pass/fail for build, boot, serial, Improv, association, outbound TCP and hostfwd.
- [ ] 5.2 If all required checks pass, add `ESP32-qemu` as a target of the integration bench with its own marker and document it; verify the target selection rejects other chips.
- [ ] 5.3 If the WiFi model is insufficient, document the limitation and add the upstream Espressif QEMU boot/serial fallback; verify the fallback report states that WiFi/MQTT/web were not exercised.
- [ ] 5.4 Update `tests/README.md` with the ESP32 QEMU tier, its limitations (RF, Bluetooth, I2C/display, timing) and the on-hardware tier that owns them.

## Workflow follow-up

- Wire the ESP32 QEMU tier into CI only after the spike passes and the runtime cost is known.
- Keep the fork commit and build recipe pinned; re-evaluate on a firmware or emulator change.
