# Integration tests (backend-neutral)

One test suite, several platforms. Tests talk to a `TargetBackend`, so the same
`tests/integration/tests/*.py` files run against esp-emulator (ESP32-S3/C3) or QEMU
(classic ESP32) with only `--target` changing.

## Targets

| `--target` | Chip | Backend | Status |
|---|---|---|---|
| `esp32s3-emu` | ESP32-S3 | esp-emulator | supported |
| `esp32c3-emu` | ESP32-C3 | esp-emulator | supported (needs a C3 build) |
| `esp32-qemu` | classic ESP32 | QEMU fork with `esp32_wifi` | boot/serial only (WiFi spike failed, see `tests/emu-esp32/SPIKE.md`) |

## Setup

From the repository root:

```bash
python3 -m venv --system-site-packages tests/integration/.venv
tests/integration/.venv/bin/pip install -r tests/integration/requirements.txt
```

Install the pinned emulator once:

```bash
curl -fsSL https://raw.githubusercontent.com/espressif/esp-emulator/main/install.sh | sh
export TINYGS_ESP_EMU="$HOME/.local/bin/esp-emu"
```

The ESP32-S3 backend uses `.pio/build/ESP32-S3/firmware.factory.bin`; build it first:

```bash
pio run -e ESP32-S3
```

For the classic ESP32 backend, build the QEMU image and the merged firmware image:

```bash
docker build -t tinygs-qemu-esp32 -f tests/emu-esp32/Dockerfile tests/emu-esp32
pio run -e ESP32
tests/integration/.venv/bin/python tests/emu-esp32/make_image.py
```

## Run

The same tests, one target at a time:

```bash
TINYGS_ESP_EMU=$HOME/.local/bin/esp-emu \
  tests/integration/.venv/bin/pytest tests/integration -q --target esp32s3-emu

TINYGS_ESP_EMU=$HOME/.local/bin/esp-emu \
  tests/integration/.venv/bin/pytest tests/integration -q --target esp32c3-emu

tests/integration/.venv/bin/pytest tests/integration -q --target esp32-qemu
```

`TINYGS_TARGET` sets the default target, so CI can export it once.

Run only tests that need a capability:

```bash
pytest tests/integration -q --target esp32-qemu -m "requires_web"
```

Capabilities are declared per target in `conftest.py`. A test marked
`@pytest.mark.requires("web")` is skipped with an explicit reason when the selected
backend does not provide it; it is never reported as passed.

## Layout

```
tests/integration/
  backends/
    base.py          # TargetBackend interface, capability names, read helpers
    esp_emu.py       # ESP32-S3/C3 backend
    qemu_esp32.py    # classic ESP32 backend
  conftest.py        # --target selection, target fixture, capability skips
  tests/
    test_boot.py             # shared boot/serial/reset tests
    test_backend_contract.py # interface contract, no target needed
  artifacts/         # backend logs per run (ignored)
```

## Backend contract

`TargetBackend` exposes `start()`, `stop()`, `reset()`, `open_serial()`, `web_url`,
`capabilities` and `log_path`. Tests must not inspect QEMU or esp-emulator flags.
Backend-specific probes belong in the backend, not in shared tests.

To add a backend, implement `TargetBackend` under `backends/`, add it to `TARGETS`
and `TARGET_CAPABILITIES` in `conftest.py`, and verify `test_boot.py` passes with
`--target <name>`.

## Scope and limitations

- Boot, serial console and reset on every target.
- WiFi/MQTT/web are not covered: the QEMU fork's `esp32_wifi` model cannot carry the
  Arduino WiFi stack (no association), and an ESP-IDF control associates but gets no DHCP
  lease. `test_wifi.py` stays skipped by capability. Details:
  `tests/emu-esp32/SPIKE.md`.
- No RF/LoRa, Bluetooth, display fidelity or real-time behaviour.
- No project source changes: the test-only radio bypass is deferred until that change is
  approved.
- On-hardware and RF validation remain a separate tier.
