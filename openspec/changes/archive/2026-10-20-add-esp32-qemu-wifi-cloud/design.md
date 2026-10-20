# Design

## Context

The boot/serial QEMU tier is already delivered (`esp32-qemu-integration-tests`,
`tests/emu-esp32/SPIKE.md`). This change investigated whether the pinned community fork's
`esp32_wifi` model can add WiFi/MQTT/web to the classic ESP32 tier. Project sources were
frozen; no firmware flag was added.

## Findings

| Experiment | Result |
|---|---|
| tinyGS provisioning over Improv | Firmware prints `Wifi status: 0` and `Failed to connect to wifi...` |
| Arduino WiFi scan control | Sees all six hardcoded APs, including `Open Wifi` (channel 4, open) |
| Arduino `WiFi.begin("Open Wifi", "", 4)` | No association (status 0) |
| ESP-IDF 5.4 station control | Associates on channel 4, but no `IP_EVENT_STA_GOT_IP`; DHCP fails |
| `-device esp32_wifi,netdev=...` | Rejected: "Parameter 'driver' expects a pluggable device type" |
| NVS/PHY calibration | IDF falls back to full calibration and still associates; not the blocker |
| `EM_RD NVDS` log | Bluetooth exchange-memory debug output; unrelated to WiFi |

Root causes to fix if the model is revisited:

1. **DHCP:** `Esp32_WLAN_can_receive()` accepts frames only in
   `STATE_ASSOCIATED`/`STATE_STA_ASSOCIATED`. After association the model sends a DHCP
   discover and moves to `STATE_STA_DHCP`, so slirp's DHCP reply is dropped. The DHCP
   offer parser may also need verification.
2. **Arduino association:** the association response uses a fixed capability field and a
   small IE set; the Arduino 2.x/IDF 4.4 driver may reject what IDF 5.4 tolerates. This
   needs frame-level debugging.
3. **Radio:** even with association and DHCP fixed, a configured boot still blocks in
   radio init; reaching MQTT would need a test-only `TINYGS_EMU_NO_RADIO` flag.

## Decision

Close the change as a negative result. Do not implement WiFi/MQTT/web on this backend and
do not claim those capabilities. Keep the classic ESP32 QEMU tier at boot/serial/reset;
keep WiFi/MQTT/web on hardware.

Rationale: the model fails before the first useful network step and fixing it is a
test-dependency project with uncertain payoff. The investigation is fully recorded, so a
future change can start from the evidence instead of repeating it.

## Revisit criteria

Open a new change if the QEMU fork gets a working DHCP path and Arduino association, or if
someone commits to patching the model. Start with `Esp32_WLAN_can_receive()` and the
association response, then re-run the Arduino and IDF controls, and only then evaluate the
test-only radio bypass.

## Evidence

- `tests/emu-esp32/SPIKE.md`
- `tests/integration/artifacts/wifi-repro/`
- `tests/integration/artifacts/wifi-scan/`
- `tests/integration/artifacts/wifi-connect2/`
- `tests/integration/artifacts/idf-wifi/`
