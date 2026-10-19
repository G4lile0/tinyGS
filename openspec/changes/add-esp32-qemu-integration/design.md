# Design

## Context

See `proposal.md` for motivation. Constraints found while evaluating options:

- `espressif/esp-emulator` does not model the classic ESP32.
- The Espressif QEMU fork (`espressif/qemu`, `esp-develop`) has an `esp32` machine and boots
  a merged image, but its feature matrix marks Wi-Fi, Bluetooth, I2C and the GPIO matrix as
  unsupported. Networking is available only through the OpenCores Ethernet MAC (`open_eth`),
  which tinyGS does not use.
- The community fork `Ebiroll/qemu_esp32` (main development tree `Ebiroll/calib`, QEMU 9.2
  base, February 2026) adds an `esp32_wifi` NIC model with slirp networking, Bluetooth VHCI
  and T-Display support. Its documented invocation is:
  `-nic user,model=esp32_wifi[,hostfwd=...]`.
- Wokwi simulates the classic ESP32 with a virtual AP (`Wokwi-GUEST`), internet access and a
  CLI, but it is a hosted, token-based service with traffic limits.
- The firmware image is a merged 4 MB image; the release workflow already builds one with
  `esptool merge_bin` (bootloader `0x1000`, partitions `0x8000`, `boot_app0` `0xe000`, app
  `0x10000`).

## Goals / Non-Goals

**Goals:**

- Determine, with a short spike, whether the classic ESP32 can run tinyGS with working WiFi
  in a self-hosted QEMU.
- If yes, reuse the integration bench structure: serial over TCP, flash copies, host port
  forwarding, opt-in test servers, evidence capture.
- Keep the decision reproducible by pinning the fork commit and the build recipe.

**Non-Goals:**

- RF/LoRa, Bluetooth, display fidelity or real-time performance.
- Replacing the S3/C3 esp-emulator path or the unit suite.
- Making Wokwi the default infrastructure.
- Production endpoints or accounts.

## Decisions

### 1. Primary candidate: Ebiroll's fork with `esp32_wifi`

Use `Ebiroll/qemu_esp32` (and the `Ebiroll/calib` tree it points to) at a pinned commit.
It is the only self-hosted option found that claims WiFi emulation for the classic ESP32.

Alternatives rejected as the primary path:

- **Espressif QEMU**: no Wi-Fi, so it cannot exercise the tinyGS AP/station, MQTT or web
  flows; kept as a fallback for boot/serial only.
- **Wokwi**: hosted, token-based, traffic limits; useful for manual exploration but not a
  self-hosted CI dependency.

### 2. Build in a pinned container, run headless

Build the fork in a Docker image with the exact commit and `--target-list=xtensa-softmmu`,
`--enable-slirp`, without GTK/SDL. Run with `-nographic`. This keeps the host clean and the
build reproducible, and avoids the display backend issues noted by the fork.

The image is cached by commit + build options. The first build is the expensive step; the
spike records its duration and disk size.

### 3. Console, monitor and control

- UART: `-serial tcp::<uart-port>,server,nowait`; the test connects with `pyserial` over
  `socket://`.
- QEMU monitor: `-monitor tcp::<monitor-port>,server,nowait` for `system_reset`, `quit` and
  memory/flash inspection.
- Flash: `-drive file=<flash-copy>.bin,if=mtd,format=raw`.
- Disable the timer watchdog as documented by Espressif:
  `-global driver=timer.esp32.timg,property=wdt_disable,value=true`.

There is no `--control-tcp` equivalent; reset goes through the QEMU monitor, and each test
gets its own process and flash copy.

### 4. Image and flashing

Use the `ESP32` merged image (`firmware.factory.bin`, or the release `merge_bin` equivalent).
For migration-style tests, write only `bootloader` at `0x1000`, `partitions` at `0x8000`
and the app at `0x10000`, leaving NVS at `0x9000` and otadata at `0xe000` intact; do not
write `boot_app0.bin` unless a clean OTA slot is intended.

The image is copied per test under a temporary directory; QEMU writes to the copy.

### 5. WiFi and the device web

Use `-nic user,model=esp32_wifi` (plus `hostfwd` for the device web). The firmware is
provisioned over Improv serial, then joins the emulated network. If the NIC model supports
only station mode, that is sufficient: the first-boot AP mode is not required because Improv
writes the credentials directly. Verify:

1. association and IPv4 address;
2. outbound TCP to the test MQTT broker (TLS 8883);
3. `hostfwd=tcp::<host-port>-:80` serving the configuration page.

If the model does not support `hostfwd` to the guest, the web test is dropped and the
limitation is recorded; it is not replaced by a fake HTTP server.

### 6. Radio bypass

Reuse the test-only `TINYGS_EMU_NO_RADIO` build flag introduced by the emulator bench. A
configured classic ESP32 image would otherwise block in SX126x initialisation because QEMU
does not model the external SPI radio. The flag is never part of a release environment.

### 7. Test-server safety

As with the other integration tier: cloud tests are opt-in, use a dedicated test station,
refuse production hosts, delete created stations in `finally`, and keep runs below the
one-hour auto-OTA interval with a serial watchdog.

### 8. Spike go/no-go

The change is gated by a spike, not by an assumption. It passes only if all of these hold
three times in a row:

1. the pinned fork builds in the container;
2. the `ESP32` image boots to the TinyGS banner;
3. Improv provisioning is accepted;
4. the firmware associates and obtains an address through `esp32_wifi`;
5. an outbound TCP connection reaches the test broker;
6. the device web is reachable through `hostfwd` (or is explicitly waived as a known
   limitation);
7. the run is stable enough to finish within the tier's time budget.

If the spike fails, the fallback is the upstream Espressif QEMU for boot/serial only, and
WiFi/MQTT/web coverage for the classic ESP32 stays on hardware. The result is recorded in
the change before any further integration work.

## Risks / Trade-offs

- [Community fork, no releases or long-term support] → Pin a commit, build in a container,
  record the commit and keep the fallback.
- [The WiFi model may be station-only or incomplete] → Improv avoids AP mode; mark web/
  hostfwd as conditional; do not fake coverage.
- [QEMU build is heavy] → Cache the image by commit; build once in CI; keep the spike
  time-boxed.
- [No RF, I2C/OLED or real timing] → Explicitly out of scope; the hardware tier owns them.
- [Auto-OTA] → `restrict`-like network policy is not available in the same form; use a
  watchdog and short runs, and avoid the production OTA host.
- [Fork diverges from QEMU/ESP-IDF] → Pin the commit; re-evaluate only when the firmware or
  the test needs change.

## Migration Plan

Additive and gated: a Docker recipe, a pinned fork build and a test bench target. If the
spike fails, only the documentation and the spike result remain; no runtime dependency is
added. If it passes, the classic ESP32 target joins the existing integration tier, and the
hardware tier keeps RF validation.

## Open Questions

- Does the `esp32_wifi` model support `hostfwd` to a guest HTTP server, or only outbound
  connections? The spike answers this.
- Does the model require a specific QEMU machine option or firmware-side NIC initialization?
  The spike answers this from the fork's examples.
