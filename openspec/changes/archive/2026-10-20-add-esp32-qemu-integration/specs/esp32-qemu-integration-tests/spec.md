# Spec Delta

## Purpose

Provides a hardware-free boot, serial and reset path for the classic ESP32 environment
using a pinned QEMU fork, behind the same backend-neutral target interface as the other
emulator targets. WiFi, MQTT and the device web are out of scope for this capability.

## ADDED Requirements

### Requirement: The same tests run on every backend

The integration suite SHALL define test cases against a common target interface and SHALL
run the same test files against every supported backend, with target selection changing
only the backend. The initial backends SHALL be esp-emulator (ESP32-S3/C3) and QEMU
(ESP32).

#### Scenario: Same boot test on two backends

- **WHEN** the suite runs the boot test with the esp-emulator S3 target and with the ESP32
  QEMU target
- **THEN** the same test file runs in both cases without test-code changes

#### Scenario: Capability-specific test

- **WHEN** a test requires a capability that the selected backend does not declare
- **THEN** the test is skipped with an explicit reason and is not reported as passed

#### Scenario: Backend details stay in the backend

- **WHEN** a test needs a serial stream, a reset or a flash image
- **THEN** it obtains them from the target interface, not from QEMU or emulator flags

### Requirement: The QEMU fork is pinned and built reproducibly

The bench SHALL build a pinned commit of the ESP32 QEMU fork inside a container image and
SHALL record the commit, build options, image size and build cost.

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
- **THEN** the console shows the TinyGS banner, the chip model, the OTP and the control
  menu, with no panic, abort or watchdog reset

#### Scenario: Defective image

- **WHEN** the firmware panics, resets repeatedly or never prints its banner within the
  boot timeout
- **THEN** the test fails and keeps the console and QEMU log as evidence

### Requirement: Serial console is reachable over TCP

The bench SHALL expose the emulated UART on a TCP port and SHALL use it for console output
and input.

#### Scenario: Console command

- **WHEN** a test sends a firmware control command over the UART bridge
- **THEN** the command is executed and its response appears in the captured console

### Requirement: QEMU monitor reset and quit

The bench SHALL expose the QEMU monitor on a TCP port and SHALL use it to reset the guest
and to stop the run.

#### Scenario: Reset without restarting the process

- **WHEN** a test sends `system_reset` on the monitor
- **THEN** the guest boots again and the UART stream can be read again

#### Scenario: Quit

- **WHEN** the fixture stops the backend
- **THEN** `quit` terminates the QEMU process and the backend waits for it

### Requirement: Runs are isolated and produce evidence

Each test SHALL use its own QEMU process, ports and flash image copy, and a failure SHALL
retain the console and QEMU log.

#### Scenario: Parallel runs

- **WHEN** two QEMU tests run at the same time
- **THEN** each uses distinct ports and flash state and neither observes the other

#### Scenario: Failure evidence

- **WHEN** a boot or serial assertion fails
- **THEN** the console and QEMU log are retained under the run artifacts

### Requirement: Scope and limitations are documented

The documentation SHALL state that the QEMU tier covers boot, serial and reset only, and
SHALL name the WiFi/MQTT/web follow-up and the on-hardware tier.

#### Scenario: Contributor chooses a tier

- **WHEN** a contributor reads the test documentation
- **THEN** it is clear whether a new test belongs in the unit, boot/serial QEMU,
  esp-emulator or on-hardware tier
