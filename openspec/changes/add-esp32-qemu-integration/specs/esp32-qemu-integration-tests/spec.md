# Spec Delta

## Purpose

Provides a hardware-free integration path for the classic ESP32 environment using a pinned
QEMU fork with WiFi emulation, so tinyGS can be exercised without a physical board while RF
behaviour remains out of scope.

## ADDED Requirements

### Requirement: The QEMU fork is pinned and built reproducibly

The bench SHALL build a pinned commit of the ESP32 QEMU fork inside a container image and
SHALL record the commit and build options used.

#### Scenario: Reproducible build

- **WHEN** two clean hosts build the bench image from the same commit and recipe
- **THEN** both produce a QEMU binary that boots the same firmware image

#### Scenario: Fork not available

- **WHEN** the pinned commit cannot be fetched or built
- **THEN** the bench reports a clear infrastructure error before running any test

### Requirement: The classic ESP32 boots from a merged image

The bench SHALL start QEMU with `-machine esp32` and the merged 4 MB flash image for the
`ESP32` environment, and SHALL capture the firmware console.

#### Scenario: Healthy boot

- **WHEN** the merged `ESP32` image is started
- **THEN** the console shows the TinyGS banner, the chip model and the control menu, with no
  panic, abort or watchdog reset

#### Scenario: Defective image

- **WHEN** the firmware panics, resets repeatedly or never prints its banner within the
  boot timeout
- **THEN** the test fails and keeps the console and QEMU log as evidence

### Requirement: Serial console is reachable over TCP

The bench SHALL expose the emulated UART on a TCP port and SHALL use it for console input,
output and Improv provisioning.

#### Scenario: Console command

- **WHEN** a test sends a firmware control command over the UART bridge
- **THEN** the command is executed and its response appears in the captured console

#### Scenario: Improv provisioning

- **WHEN** the bench sends valid WiFi credentials over the UART bridge
- **THEN** the firmware acknowledges them and connects to the emulated WiFi network

### Requirement: WiFi emulation supports the tinyGS flow

The bench SHALL use the fork's `esp32_wifi` NIC model and SHALL verify that the firmware
obtains an address and can reach a host-exposed service.

#### Scenario: Station connection

- **WHEN** the firmware is provisioned with the bench SSID and password
- **THEN** it associates with the emulated network and obtains an IPv4 address

#### Scenario: Device web console

- **WHEN** the bench forwards a host port to the device's HTTP port
- **THEN** the tinyGS configuration page is reachable on that host port

#### Scenario: Outbound connection

- **WHEN** the firmware opens a TCP connection to a test host
- **THEN** the connection is carried by the user-mode network backend

### Requirement: Test-server use is opt-in and non-production

The bench SHALL contact `mqtt-test.tinygs.com` or `app-test.tinygs.com` only when the test
account configuration is present and cloud tests are explicitly enabled, and SHALL never
target production endpoints.

#### Scenario: Cloud enabled

- **WHEN** the test credentials are present and cloud tests are enabled
- **THEN** the station connects to the test broker and is exercised against the test web

#### Scenario: Production endpoint

- **WHEN** a configuration points at a production tinyGS endpoint
- **THEN** the bench refuses to run the cloud tests and names the offending setting

### Requirement: Configured boot does not block on the absent radio

The bench SHALL use the documented test-only radio bypass so a configured image reaches its
network logic when no radio is attached, and the bypass SHALL be absent from release builds.

#### Scenario: No radio attached

- **WHEN** a configured ESP32 image boots under QEMU without a radio
- **THEN** the firmware reaches its WiFi and MQTT logic instead of blocking in radio
  initialisation

#### Scenario: Release build unaffected

- **WHEN** a release environment is built
- **THEN** the bypass is not defined and radio initialisation behaves as on hardware

### Requirement: A failed spike has a documented fallback

If the fork cannot boot the firmware with WiFi, or its WiFi model cannot carry the required
flows, the bench SHALL record the result and fall back to boot/serial coverage with the
upstream Espressif QEMU fork, leaving WiFi/MQTT/web coverage on hardware.

#### Scenario: WiFi model insufficient

- **WHEN** the spike proves the firmware cannot associate or reach a host through the NIC
- **THEN** the documentation records the limitation and the classic ESP32 keeps its
  on-hardware coverage

#### Scenario: Fallback run

- **WHEN** the fallback is used
- **THEN** the test explicitly reports that only boot and serial behaviour were exercised

### Requirement: Runs are isolated and produce evidence

Each test SHALL use its own QEMU process, ports and flash image copy, and a failure SHALL
retain the console, QEMU log and any web evidence.

#### Scenario: Parallel runs

- **WHEN** two QEMU tests run at the same time
- **THEN** each uses distinct ports and flash state and neither observes the other

#### Scenario: Failure evidence

- **WHEN** a boot, WiFi or web assertion fails
- **THEN** the console, QEMU log and relevant screenshot or page content are retained

### Requirement: Scope is documented

The documentation SHALL state that the QEMU tier does not cover RF, Bluetooth, real timing
or physical board identity, and SHALL name the hardware tier that does.

#### Scenario: Contributor chooses a tier

- **WHEN** a contributor reads the test documentation
- **THEN** it is clear whether a new test belongs in the unit, S3/C3 emulator, ESP32 QEMU or
  on-hardware tier
