# Tests

Three independent tiers, in order of cost:

| Tier | Location | Runs on | Covers |
|---|---|---|---|
| Unit | `tests/unit/` | host | pure C/C++ logic (Golay-24 smoke test today) |
| Integration | `tests/integration/` | esp-emulator / QEMU | boot, serial, reset; WiFi/MQTT/web where the backend supports it |
| On-hardware | separate | physical board | RF, radio, board identity and real timing |

## Unit

```bash
cmake -S tests/unit -B tests/unit/build
cmake --build tests/unit/build
ctest --test-dir tests/unit/build --output-on-failure
```

See `tests/unit/README.md`.

## Integration

One suite, several targets; the same test files run on each backend:

```bash
python3 -m venv --system-site-packages tests/integration/.venv
tests/integration/.venv/bin/pip install -r tests/integration/requirements.txt

TINYGS_ESP_EMU=$HOME/.local/bin/esp-emu \
  tests/integration/.venv/bin/pytest tests/integration -q --target esp32s3-emu

tests/integration/.venv/bin/pytest tests/integration -q --target esp32-qemu
```

See `tests/integration/README.md` for targets, capabilities and the `--target` matrix.

### Classic ESP32 QEMU

The community QEMU fork with `esp32_wifi` builds reproducibly and boots the classic ESP32
for boot/serial/reset tests. Its WiFi model is not usable with tinyGS: the Arduino WiFi
stack does not associate, and an ESP-IDF control associates but gets no DHCP lease. The
`EM_RD NVDS` log is Bluetooth debug noise, not the WiFi failure. WiFi/MQTT/web stay on
hardware. Full result: `tests/emu-esp32/SPIKE.md`.

## Boundaries

- Unit: no board, emulator, serial port or network.
- Integration: no RF/LoRa, Bluetooth, display fidelity or real-time behaviour.
- No project source changes are made by the test infrastructure. A configured QEMU boot
  still blocks in radio init; a test-only bypass would need a separate, approved change.
- On-hardware validation owns RF, radio calibration and physical board identity.
