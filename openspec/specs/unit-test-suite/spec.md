# unit-test-suite Specification

## Purpose

Provides fast, deterministic host-side verification of firmware logic that does not depend
on ESP32 hardware, so regressions are caught before any board or emulator run.

## Requirements

### Requirement: Unit tests run on the host without target hardware

The suite SHALL build and execute the tested firmware sources with a host C/C++ compiler,
without an ESP32 board, emulator, serial port or network access.

#### Scenario: Clean host run

- **WHEN** a developer runs the documented unit-test command on a machine with CMake and a
  host C/C++ toolchain
- **THEN** the suite configures, builds and runs to completion without accessing a board,
  serial device or network service

#### Scenario: No target toolchain required

- **WHEN** the ESP32 toolchain and PlatformIO target packages are not installed
- **THEN** the unit suite still builds and runs

### Requirement: The harness compiles only pure firmware sources

The suite SHALL compile the tested firmware translation units directly and SHALL NOT
require Arduino, FreeRTOS, RadioLib or PlatformIO headers.

#### Scenario: Pure decoder source

- **WHEN** the suite builds the Golay-24 decoder and its test
- **THEN** the build succeeds with only the host compiler and standard C/C++ headers

#### Scenario: Arduino-dependent source

- **WHEN** a source that includes `Arduino.h` or `Logger.h` is added to the test target
- **THEN** the build fails clearly instead of silently pulling in a target framework

### Requirement: A representative decoder is covered by known vectors

The suite SHALL include a smoke test for the Golay-24 decoder covering a valid codeword,
correctable single- and double-bit errors, and an uncorrectable input.

#### Scenario: Valid codeword

- **WHEN** the zero codeword or the all-ones codeword is decoded
- **THEN** the decoder reports zero corrected errors and leaves the codeword unchanged

#### Scenario: Correctable errors

- **WHEN** one or two bits of the zero codeword are flipped
- **THEN** the decoder reports the number of corrected bits and returns the zero codeword

#### Scenario: Uncorrectable input

- **WHEN** the decoder receives a word that is not within the correction radius
- **THEN** it reports failure and does not report a corrected codeword

### Requirement: Failures are actionable and deterministic

A failing test SHALL produce a non-zero exit status and identify the failing case, the
expected value and the observed value. Re-running the suite SHALL produce the same result.

#### Scenario: Vector mismatch

- **WHEN** a decoder returns a different value than the test expects
- **THEN** the run fails and reports the case name, expected value and actual value

#### Scenario: Repeatability

- **WHEN** the suite runs twice on the same sources and toolchain
- **THEN** both runs report the same pass/fail result

### Requirement: Unit suite is isolated from release builds

The unit suite SHALL NOT modify release environments, firmware sources or generated
firmware artifacts. Its build directory SHALL be ignored by version control.

#### Scenario: Release build unaffected

- **WHEN** the unit suite runs and then a PlatformIO release environment is built
- **THEN** the firmware sources and generated image are unaffected by the unit run

#### Scenario: Build output ignored

- **WHEN** the suite builds into `tests/unit/build/`
- **THEN** git reports no new tracked or untracked build files from that directory

### Requirement: The suite has one documented entry point

The repository SHALL document the exact command that configures, builds and runs the unit
suite, and the command SHALL work verbatim on a clean checkout.

#### Scenario: New contributor

- **WHEN** a contributor follows `tests/unit/README.md` from a clean checkout
- **THEN** the documented command builds and runs the smoke test without extra steps

#### Scenario: Adding a test

- **WHEN** a contributor adds a new pure source and test file
- **THEN** the README explains where to add it and how to run it
