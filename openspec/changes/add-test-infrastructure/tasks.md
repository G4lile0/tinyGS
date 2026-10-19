# Tasks

## 1. Unit test harness

- [x] 1.1 Add `tests/unit/CMakeLists.txt` with C++17, C11, a `doctest` interface target and a `tinygs_unit_tests` executable; verify `cmake -S tests/unit -B tests/unit/build` configures successfully.
- [x] 1.2 Vendor the pinned doctest single header at `tests/unit/third_party/doctest.h` and record its version in the README; verify the header compiles in a test that includes it.
- [x] 1.3 Add `tests/unit/build/` to `.gitignore`; verify a build leaves no untracked build files in `git status`.

## 2. Smoke test

- [x] 2.1 Add `tests/unit/test_golay24.cpp` with a valid-codeword case (zero and all-ones), one- and two-bit correction cases, and an uncorrectable case; verify all cases pass with `ctest --output-on-failure`.
- [x] 2.2 Add the Golay source to the test target by its real path (`tinyGS/src/BitCode/golay24.cpp`); verify the test links against the firmware source, not a copy.
- [x] 2.3 Verify failure output by temporarily breaking one expectation; confirm the run exits non-zero and prints the case name, expected value and actual value, then restore it.

## 3. Documentation

- [x] 3.1 Add `tests/unit/README.md` with the exact configure/build/test commands, the doctest version, the directory layout and the rule that only pure sources can be added; verify every command runs as written.
- [x] 3.2 Document how to add a new pure source and a new test file; verify the instructions by following them for a throwaway test and removing it.

## 4. Verification

- [x] 4.1 Run the suite on a clean checkout with no PlatformIO target packages installed; verify it builds and passes with only CMake and the host compiler.
- [x] 4.2 Run the suite twice; verify identical results and that no file under `tinyGS/` or `.pio/` changed.
- [x] 4.3 Confirm the release environments are untouched: `git status` shows only `tests/unit/` and `.gitignore`, and `pio run -e ESP32` still builds.

## Workflow follow-up

- Track the ESP32 classic QEMU integration separately in `add-esp32-qemu-integration`; do not implement it in this change.
