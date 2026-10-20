# Classic ESP32 QEMU spike

QEMU backend for the classic ESP32 environment, using the community fork with ESP32
WiFi emulation.

- Repository: <https://github.com/Ebiroll/calib>
- Pinned commit: `73bee9bab5a78b0aaa584610b1564d533e7879ce`
- QEMU base: 9.2.2
- Image: `tinygs-qemu-esp32`

## Why this fork

The upstream Espressif QEMU fork boots the ESP32 but marks Wi-Fi as unsupported in its
feature matrix. This fork adds `hw/misc/esp32_wifi.c` and `esp32_wifi_ap.c`, exposing a
NIC model (`-nic user,model=esp32_wifi`) with slirp networking. That is the only
self-hosted option found that can exercise the tinyGS WiFi/MQTT flows for the classic
ESP32.

## Build

```bash
docker build -t tinygs-qemu-esp32 -f tests/emu-esp32/Dockerfile tests/emu-esp32
```

Build facts on the reference Linux x86_64 host:

- pinned commit `73bee9bab5a78b0aaa584610b1564d533e7879ce`, QEMU 9.2.2;
- first build: 350 s (the Docker layer cache makes a rebuild in ~4 s);
- image size: 240 MB;
- `qemu-system-xtensa --version` reports 9.2.2 and `-machine help` lists `esp32`.

The Dockerfile:

- fetches the pinned commit with a shallow fetch;
- builds only `xtensa-softmmu`, with SLIRP, without GTK/SDL/VNC/curses/capstone/docs;
- copies `qemu-system-xtensa` into a slim runtime image.

Check the emulator:

```bash
docker run --rm tinygs-qemu-esp32 --version
docker run --rm tinygs-qemu-esp32 -machine help | grep esp32
```

## Firmware image

QEMU needs a merged 4 MB flash image:

```bash
pio run -e ESP32
python tests/emu-esp32/make_image.py
```

The script merges `bootloader.bin` at `0x1000`, `partitions.bin` at `0x8000`,
`boot_app0.bin` at `0xe000` and `firmware.bin` at `0x10000`, padded to 4 MB.

## Run by hand

```bash
docker run --rm --network host \
  -v "$PWD/tests/emu-esp32/firmware-full.bin:/flash.bin:rw" \
  tinygs-qemu-esp32 \
  -machine esp32 -nographic \
  -drive file=/flash.bin,if=mtd,format=raw \
  -serial tcp:127.0.0.1:5555,server=on,wait=off \
  -monitor tcp:127.0.0.1:5556,server=on,wait=off \
  -global driver=timer.esp32.timg,property=wdt_disable,value=true \
  -nic user,model=esp32_wifi,hostfwd=tcp:127.0.0.1:8080-:80
```

Connect to the console with `nc 127.0.0.1 5555` or `pyserial` over
`socket://127.0.0.1:5555`. The QEMU monitor is on `5556` (`system_reset`, `quit`).

The fork's emulated APs are hardcoded (`Open Wifi`, `MasseyWifi`, ...). WiFi is **not
covered** by this tier yet: with the Arduino image the model loops on `EM_RD NVDS 0x0000`
after provisioning. See `SPIKE.md` for the result and `add-esp32-qemu-wifi-cloud` for the
follow-up.

## Notes and limitations

- The fork is community-maintained, not supported by Espressif; the commit is pinned.
- QEMU does not model the external SX126x radio, so the test image uses the test-only
  `TINYGS_EMU_NO_RADIO` flag.
- No RF, Bluetooth, I2C/OLED or real-time behaviour.
- The QEMU process writes to `flash.bin`; the integration backend always uses a copy.
