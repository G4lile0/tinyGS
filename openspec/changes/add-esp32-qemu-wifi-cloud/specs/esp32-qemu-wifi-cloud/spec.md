# Spec Delta

## Purpose

Completes the classic ESP32 QEMU tier so the shared integration tests cover WiFi, the
device web, MQTT and opt-in test-server runs, after resolving why the `esp32_wifi` model
loops on `EM_RD NVDS`.

## ADDED Requirements

### Requirement: The NVS/PHY blocker is investigated and documented

The work SHALL identify why the pinned QEMU fork's `esp32_wifi` model loops on
`EM_RD NVDS 0x0000` with the Arduino image, and SHALL document the cause or the evidence
that rules out each candidate.

#### Scenario: Reproducible symptom

- **WHEN** the classic ESP32 image is provisioned and booted with `-nic user,model=esp32_wifi`
- **THEN** the investigation captures the `EM_RD NVDS` loop and the firmware state

#### Scenario: Candidate ruled out

- **WHEN** a candidate (populated PHY/NVS, eFuse/PHY blob, machine/NIC option) is tested
- **THEN** the result is recorded as pass or fail with the exact command and artifact

### Requirement: WiFi association works after provisioning

The QEMU backend SHALL accept Improv credentials and the firmware SHALL associate with the
emulated network and obtain an IPv4 address.

#### Scenario: Provisioning

- **WHEN** the shared provisioning test sends valid SSID/password over the UART bridge
- **THEN** the firmware acknowledges the credentials and reboots

#### Scenario: Association

- **WHEN** the reboot completes
- **THEN** the firmware obtains an IPv4 address and the backend can observe the guest's
  network identity

### Requirement: Device web is reachable through hostfwd

The backend SHALL expose the device web on a host loopback port and the shared web test
SHALL read the tinyGS page.

#### Scenario: Web page

- **WHEN** the guest is associated
- **THEN** `hostfwd` returns the tinyGS configuration page on the host port

### Requirement: Outbound TCP and MQTT work

The backend SHALL carry an outbound TCP connection from the firmware and the shared MQTT
test SHALL connect to the test broker when credentials are provided.

#### Scenario: Outbound TCP

- **WHEN** the firmware opens a TCP connection to a host listener
- **THEN** the connection is accepted through the user-mode network

#### Scenario: MQTT

- **WHEN** the test MQTT credentials are present and cloud tests are enabled
- **THEN** the station connects to `mqtt-test.tinygs.com` and completes the expected
  exchange

### Requirement: Capability claims follow passing tests

The QEMU backend SHALL declare `wifi`, `web` and `mqtt` only after the corresponding shared
tests pass; until then those tests SHALL be skipped with an explicit reason.

#### Scenario: Claim enabled

- **WHEN** the WiFi, web and MQTT tests pass on QEMU
- **THEN** the capability matrix includes them and the shared tests run instead of skipping

#### Scenario: Claim not enabled

- **WHEN** a capability test is not passing
- **THEN** the backend does not claim it and the test is skipped, never reported as passed

### Requirement: A test-only radio bypass is conditional and isolated

If a configured boot still blocks in radio init, a `TINYGS_EMU_NO_RADIO`-style bypass MAY be
added only for test builds, and it SHALL be absent from release environments and on-hardware
binaries.

#### Scenario: Bypass needed

- **WHEN** the configured QEMU boot reaches WiFi but blocks before MQTT because of the
  absent radio
- **THEN** the bypass is added to a test-only environment and documented

#### Scenario: Release unaffected

- **WHEN** a release environment is built
- **THEN** the bypass is not defined and radio initialisation is unchanged

### Requirement: Cloud runs are safe and clean up

When the QEMU backend joins the cloud tier, it SHALL use the same opt-in, production-host
guard, dedicated station and cleanup rules as the other integration backend.

#### Scenario: Cloud disabled

- **WHEN** the cloud flag or credentials are missing
- **THEN** cloud tests are skipped and local WiFi/web tests still run

#### Scenario: Cleanup

- **WHEN** a test creates a station and then fails
- **THEN** the backend still attempts to delete it and reports the result

### Requirement: The full WiFi path is stable

The provisioning, association, web and MQTT path SHALL pass three consecutive runs from
fresh flash copies.

#### Scenario: Three runs

- **WHEN** the WiFi/MQTT suite runs three times
- **THEN** all three runs pass and the artifacts show no flaky failure

### Requirement: Same tests, both backends

The WiFi/MQTT/web tests SHALL remain backend-neutral and SHALL run unchanged on
esp-emulator and QEMU when both declare the capability.

#### Scenario: Cross-backend run

- **WHEN** the same test file runs with `--target esp32s3-emu` and `--target esp32-qemu`
- **THEN** no test code branches on the backend type
