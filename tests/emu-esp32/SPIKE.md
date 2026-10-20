# Spike result: classic ESP32 QEMU

Date: 2026-10-20
Host: Linux x86_64, Docker 29.9.1
Fork: `Ebiroll/calib` at `73bee9bab5a78b0aaa584610b1564d533e7879ce` (QEMU 9.2.2)
Image: `tinygs-qemu-esp32` (240 MB, first build 350 s, cached rebuild 4 s)
Firmware: merged 4 MB `ESP32` image (`tests/emu-esp32/make_image.py`)

## Checklist

| Check | Result | Evidence |
|---|---|---|
| Pinned fork builds in Docker | PASS | `qemu-system-xtensa --version` = 9.2.2; `-machine help` lists `esp32` |
| Classic ESP32 boots to TinyGS banner and controls | PASS | shared `test_boot_reaches_tinygs_console` |
| UART over TCP and console command | PASS | shared `test_console_command_is_answered` |
| QEMU monitor `system_reset` and serial reconnect | PASS | shared `test_reset_boots_the_same_firmware_again` |
| QEMU monitor `quit` stops the process | PASS | backend `stop()` waits for the container to exit |
| Improv provisioning over serial | FAIL | WiFi init loops on `EM_RD NVDS 0x0000` in the QEMU log |
| WiFi association and IPv4 address | NOT REACHED | no address or web response after provisioning |
| Outbound TCP / MQTT | NOT REACHED | WiFi never becomes usable |
| `hostfwd` to the device web | FAIL | `http://127.0.0.1:<port>/` timed out for 90 s |

## Diagnosis

The guest boots and the console works, so the CPU, flash, UART, timers and monitor are
usable. After provisioning, the WiFi driver repeatedly reads `NVDS` and never completes
initialisation. The QEMU log fills with:

```
EM_RD NVDS 0x0000 -> 0x3ffae87c
```

This points at the `esp32_wifi` model interacting with the Arduino/ESP-IDF WiFi stack or
its PHY/NVS calibration data, not at tinyGS's application logic. The model was validated
by its author against specific ESP-IDF examples; the Arduino core's WiFi/NVS layout is a
different target.

## Decision

- Keep the QEMU fork as a **boot/serial** backend for the classic ESP32. It is useful and
  stable for that.
- Do **not** claim WiFi, MQTT or device-web coverage for `esp32-qemu` yet.
- Do **not** add a test-only radio bypass or any other firmware change in this change:
  project sources are frozen for now.
- Keep WiFi/MQTT/web coverage on hardware. The upstream Espressif QEMU fork does not
  emulate WiFi, so it is not a replacement for this path.

## Next steps (needs approval)

1. Investigate the `EM_RD NVDS` loop: whether the fork needs a populated PHY/NVS partition,
   an eFuse/PHY blob, or a specific `-nic`/machine option.
2. If that works, evaluate a test-only radio bypass so a configured boot reaches MQTT.
   That requires a small firmware change and is intentionally out of scope today.
3. Only then add `web`/`mqtt` capability claims and the corresponding shared tests.
