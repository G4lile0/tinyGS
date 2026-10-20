# Design

## Context

See `proposal.md` for motivation. The current state that shapes the approach:

- `espressif/esp-emulator` does not model the classic ESP32.
- The upstream Espressif QEMU fork boots the ESP32 but its feature matrix marks Wi-Fi,
  Bluetooth and I2C as unsupported.
- The community fork `Ebiroll/calib` (QEMU 9.2.2) adds `hw/misc/esp32_wifi.c` and an
  `esp32_wifi` NIC model. The pinned commit is
  `73bee9bab5a78b0aaa584610b1564d533e7879ce`.
- The classic ESP32 image is a merged 4 MB flash image; `tests/emu-esp32/make_image.py`
  builds it from the PlatformIO outputs.
- The fork boots the tinyGS image and its serial/reset path works. Its WiFi model does not
  complete WiFi init with the Arduino image: the guest loops on `EM_RD NVDS 0x0000`.
  Details: `tests/emu-esp32/SPIKE.md`.
- Project sources are frozen for this change. No test-only firmware flag is added.

## Goals / Non-Goals

**Goals:**

- A pinned, reproducible QEMU build for the classic ESP32.
- Backend-neutral integration tests: the same boot/serial/reset files run against
  esp-emulator (S3/C3) and QEMU (ESP32).
- Clear evidence and a documented capability matrix.

**Non-Goals:**

- WiFi association, MQTT, the device web, hostfwd, cloud/test-server runs.
- The `EM_RD NVDS` investigation and any test-only radio bypass; tracked in
  `add-esp32-qemu-wifi-cloud`.
- RF/LoRa, Bluetooth, I2C/OLED or real-time behaviour.
- Firmware or release-environment changes.

## Decisions

### 1. Primary backend: the pinned community QEMU fork

Use `Ebiroll/calib` at the pinned commit. The upstream Espressif QEMU fork cannot emulate
WiFi, but for this change only boot/serial/reset matter; the community fork is the one
already built and validated. Pin the commit and build in Docker.

### 2. Backend-neutral target interface

The suite lives in `tests/integration/`:

```
tests/integration/
  backends/
    base.py          # TargetBackend protocol + capability names
    esp_emu.py       # ESP32-S3 / ESP32-C3 backend
    qemu_esp32.py    # classic ESP32 backend
  conftest.py        # --target selection, target fixture, capability skips
  tests/
    test_boot.py             # shared boot/serial/reset tests
    test_backend_contract.py # interface contract
    test_wifi.py             # future; skipped until WiFi is supported
```

`TargetBackend` exposes `start()`, `stop()`, `reset()`, `open_serial()`, `web_url`,
`capabilities` and `log_path`. Tests use only that interface. A test that needs an optional
capability uses `@pytest.mark.requires(...)`; the conftest skips it with the backend name
and the missing capability. Backend-specific probes stay in the backend.

`--target esp32s3-emu`, `--target esp32c3-emu` and `--target esp32-qemu` select the
backend. `tests/integration/tests/test_boot.py` is the same file for all three.

### 3. Build in a pinned container, run headless

The Dockerfile fetches the commit with a shallow fetch, builds only `xtensa-softmmu` with
SLIRP, without GTK/SDL/VNC/curses/capstone/docs, and copies the binary and `pc-bios` into a
slim runtime image. Results: 240 MB image, 350 s first build, ~4 s cached rebuild.

### 4. Console, monitor and flash

- UART: `-serial tcp:127.0.0.1:<port>,server=on,wait=off`, read with `pyserial` over
  `socket://`.
- Monitor: `-monitor tcp:127.0.0.1:<port>,server=on,wait=off`; `system_reset` resets the
  guest, `quit` stops the process.
- Flash: a per-test copy of the merged image passed with
  `-drive file=<copy>,if=mtd,format=raw`.
- Watchdog: `-global driver=timer.esp32.timg,property=wdt_disable,value=true`.
- The NIC is enabled with `-nic user,model=esp32_wifi` for the spike, but no test relies on
  it in this change.

### 5. Images and isolation

`tests/emu-esp32/make_image.py` merges `bootloader.bin` at `0x1000`, `partitions.bin` at
`0x8000`, `boot_app0.bin` at `0xe000` and `firmware.bin` at `0x10000`, padded to 4 MB.
Each test copies it under its artifact directory; QEMU writes only to the copy. Tests get
distinct free ports and one process each.

### 6. Scope: boot/serial/reset only

The QEMU backend declares `serial`, `reset`, `flash` and `improv`; it does not declare
`wifi`, `web` or `mqtt`. The shared WiFi test is present but skipped with an explicit
reason. This keeps the suite green while preserving the future test.

## Risks / Trade-offs

- [Community fork, no long-term support] → Pin the commit, build in Docker, record the
  recipe and keep the boot/serial scope.
- [QEMU build is heavy] → Cache the image by commit; first build ~350 s, rebuild ~4 s.
- [No WiFi coverage] → Explicitly documented; the follow-up change owns it.
- [No firmware changes] → A configured boot still blocks in radio init; this tier tests
  the unconfigured boot and serial path only.
- [No RF, I2C/OLED or real time] → Out of scope; on-hardware validation owns them.

## Migration Plan

Additive: `tests/emu-esp32/`, `tests/integration/`, documentation and Docker. No firmware,
release environment or generated artifact changes. Rollback is deleting the test tree and
the `.gitignore` entries.

## Open Questions

None for this change. The WiFi/NVS investigation and the radio-bypass decision are tracked
in `add-esp32-qemu-wifi-cloud`.
