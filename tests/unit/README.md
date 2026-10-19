# Host unit tests

Fast, hardware-free tests for firmware code that does not depend on Arduino, FreeRTOS,
RadioLib or PlatformIO. The suite builds with a host C/C++ compiler and runs in
milliseconds.

## Requirements

- CMake 3.16 or newer
- A C++17 compiler and a C11 compiler (GCC or Clang)

No ESP32 toolchain, board, serial port, emulator or network access is required.

## Run the suite

From the repository root:

```bash
cmake -S tests/unit -B tests/unit/build
cmake --build tests/unit/build
ctest --test-dir tests/unit/build --output-on-failure
```

The build directory is `tests/unit/build/` and is ignored by git. To rebuild from scratch:

```bash
rm -rf tests/unit/build
```

To run the test binary directly (for doctest filtering):

```bash
./tests/unit/build/tinygs_unit_tests
./tests/unit/build/tinygs_unit_tests --test-case="Golay-24 corrects*"
./tests/unit/build/tinygs_unit_tests --list-test-cases
```

## Layout

```
tests/unit/
  CMakeLists.txt          # test executable and CTest registration
  test_main.cpp           # doctest main
  test_golay24.cpp        # smoke test
  third_party/doctest.h   # pinned doctest 2.4.11 (MIT)
```

The test target compiles the firmware source directly, for example
`tinyGS/src/BitCode/golay24.cpp`, so there is no copy that can drift from the firmware.

## Add a test

1. Add a `test_<name>.cpp` that includes `doctest.h` and the header under test.
2. Add the test file and the firmware source it needs to `add_executable(...)` in
   `CMakeLists.txt`.
3. Add the source's include directory to `target_include_directories(...)` if needed.
4. Run the commands above.

Only pure sources can be added. If a source includes `Arduino.h`, `Logger.h`, FreeRTOS or
RadioLib, the host build will fail: that is intentional. Either test a pure helper or
extract the pure logic into a separate translation unit first.

## Scope

This suite covers pure logic only. Integration tests (emulator/QEMU, serial, WiFi, MQTT,
device web) are separate. RF and on-hardware behaviour are not covered here.

## Test framework

[doctest](https://github.com/doctest/doctest) 2.4.11, vendored as a single header under
`third_party/`. It is MIT-licensed; the license text is in the header.

## Current coverage

- Golay-24 decoder (`decode_golay24`): valid codewords, one- and two-bit corrections,
  uncorrectable input.
