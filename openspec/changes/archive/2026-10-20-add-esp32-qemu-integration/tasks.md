# Tasks

## 1. Pin and build the fork

- [x] 1.1 Add a Docker recipe under `tests/emu-esp32/` that clones the QEMU fork at a pinned commit and builds `xtensa-softmmu` with SLIRP and without GTK/SDL; verify `qemu-system-xtensa --version` and `-machine esp32` are available in the image.
- [x] 1.2 Document the pinned commit, build options, image size and first-build duration; verify a second build from the cache is significantly faster.

## 2. Backend-neutral suite

- [x] 2.1 Define the `TargetBackend` interface and capability names in `tests/integration/backends/base.py`; verify a contract test can start, stop and read capabilities from a fake backend.
- [x] 2.2 Implement the esp-emulator backend for `ESP32-S3` and `ESP32-C3` (`--uart-tcp`, `--control-tcp`, factory image) against the interface; verify the shared boot test passes on the S3 backend.
- [x] 2.3 Add `--target` selection and a `target` fixture in `tests/integration/conftest.py`; verify an unknown target fails with the supported list and a capability-specific test is skipped with a reason.
- [x] 2.4 Add the shared `tests/integration/tests/test_boot.py` (banner, chip, OTP, control menu, no panic) and verify it runs unchanged on the esp-emulator S3 backend.

## 3. Boot and serial spike

- [x] 3.1 Build the `ESP32` merged 4 MB image (or use the release `firmware.full.bin`) and boot it with `-machine esp32`, `-nographic` and the MTD flash drive; verify the console reaches the TinyGS banner and control menu.
- [x] 3.2 Expose UART with `-serial tcp::<port>,server,nowait` and connect with `pyserial`; verify a console command and its response, and that the stream survives a QEMU monitor `system_reset`.
- [x] 3.3 Expose the QEMU monitor on a TCP port and verify `system_reset` and `quit` terminate or restart the run as expected.

## 4. Scope decision and documentation

- [x] 4.1 Run the boot/serial suite three times on QEMU and verify 3/3 passes.
- [x] 4.2 Keep the classic ESP32 QEMU target as boot/serial/reset only and set its capability matrix accordingly; verify WiFi/web/MQTT tests are skipped with a reason.
- [x] 4.3 Update `tests/README.md` and `tests/integration/README.md` with the QEMU tier, its limitations and the on-hardware boundary; verify the documented commands run.
- [x] 4.4 Record the WiFi/NVS blocker in `tests/emu-esp32/SPIKE.md` and open the follow-up change `add-esp32-qemu-wifi-cloud`.

## Workflow follow-up

- Archive this change after review.
- The deferred WiFi/MQTT/web/cloud work is tracked in `add-esp32-qemu-wifi-cloud`.
