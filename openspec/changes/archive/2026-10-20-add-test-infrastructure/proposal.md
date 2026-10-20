# Proposal

## Why

The firmware has no automated unit tests. Logic that is pure C/C++ — starting with the
satellite frame decoders — can be compiled and tested on the host in milliseconds, without
a board, emulator, serial port or network. Establishing that harness now gives the project
a fast regression signal and a place to grow coverage. Integration tests (emulator or
hardware) are a separate concern and a separate change.

## What Changes

- Add a host-native unit test project in `tests/unit/` built with CMake and CTest.
- Vendor a pinned, single-header test framework (doctest) so the suite builds offline.
- Compile only the firmware translation units that have no Arduino/RadioLib dependency.
- Add a small smoke test for the Golay-24 decoder as the proof that the harness works.
- Add a single documented command to configure, build and run the suite, plus a README.
- Keep test build output out of version control.

Out of scope for this change: integration tests, emulator/QEMU runs, test servers, CI
workflows, RF/hardware coverage, and refactoring the firmware beyond what the tested
translation units already require. The classic ESP32 QEMU integration path is tracked
separately in `add-esp32-qemu-integration`.

## Capabilities

### New Capabilities

- `unit-test-suite`: host-native, deterministic tests for pure firmware logic, runnable with
  one command and isolated from release builds.

### Modified Capabilities

None. The change is additive and does not alter firmware behaviour.

## Impact

- New code: `tests/unit/` (CMake project, vendored doctest header, smoke test, README).
- New development dependency: a host C/C++ toolchain and CMake; no new firmware
  dependency and no runtime change.
- `.gitignore` ignores the unit test build directory.
- Release environments, binaries and the firmware source tree are unchanged.
