# Design

## Context

See `proposal.md` for motivation. The current state that shapes the approach:

- The firmware is an Arduino/PlatformIO project (`src_dir = tinyGS`). Its core is
  `tinyGS.ino` plus `tinyGS/src/*`; most modules include `Arduino.h`, FreeRTOS or RadioLib.
- A few decoding units are already pure C/C++: `BitCode/golay24.cpp`,
  `BitCode/randomizer.cpp`, `BitCode/ax100_mode5.cpp` and `BitCode/decode_rs_8.cpp`.
- `BitCode/BitCode.cpp` includes `Logger.h`, which pulls in `Arduino.h` and FreeRTOS, so it
  cannot be compiled on the host without a seam or an extraction.
- `test/` is the PlatformIO test directory and is currently only the placeholder README.
  PlatformIO's `native` environment would inherit `src_dir = tinyGS` and try to build the
  Arduino sources, so it is not a zero-friction host harness.
- The CI build workflow only builds release environments; there is no test job.

## Goals / Non-Goals

**Goals:**

- A one-command host suite that builds in seconds and needs only CMake and a C/C++ compiler.
- A real smoke test that exercises a decoder, not a placeholder assertion.
- A layout that makes adding more pure sources and vectors mechanical.
- Zero changes to firmware sources or release environments for this first slice.

**Non-Goals:**

- Integration tests, emulator/QEMU, test servers or CI wiring in this change.
- Mocking Arduino, FreeRTOS or RadioLib.
- Covering every decoder now; the smoke test proves the harness, later changes add vectors.
- Moving or refactoring `BitCode.cpp` or other Arduino-dependent modules.

## Decisions

### 1. CMake + CTest, not a PlatformIO native environment

`tests/unit/CMakeLists.txt` is a standalone project that compiles the selected pure sources
and the test files. `ctest` runs the resulting executables.

Rationale: it does not inherit `src_dir = tinyGS`, so it cannot accidentally compile
Arduino code; it works on any machine with CMake and a host compiler; and it does not
interfere with PlatformIO. A PlatformIO `native` environment would need a source filter or
a mock layer and is brittle as files move.

Alternatives considered: `pio test -e native` (rejected for the `src_dir`/Arduino reason);
a bespoke build script (rejected: CMake is already available and gives dependency tracking).

### 2. Vendored doctest, pinned

Vendor the single-header doctest release under `tests/unit/third_party/doctest.h` and pin
its version in the README. Tests use `TEST_CASE`/`CHECK`, and one translation unit defines
`DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`.

Rationale: no network at configure time, no package manager, one header, fast compile and
readable failure output. It is MIT-licensed and the header carries its license text.

Alternatives considered: GoogleTest/Catch2 via `FetchContent` (heavier, network/cache
dependent); a custom assert runner (no filtering/reporting, reinvents the basics).

### 3. Compile only pure sources, explicitly

The test target lists each source it needs instead of globbing or building the whole
`tinyGS/src` tree. The first target is:

```
tests/unit/test_golay24.cpp
tinyGS/src/BitCode/golay24.cpp
```

If a future test needs `BitCode.cpp`, the plan is to extract its pure helpers into a
`BitCodeCore` unit without the `Logger.h` include, then compile that unit. Until then,
adding an Arduino-dependent source to the target must fail at compile time, which is the
intended feedback.

### 4. Smoke test scope: Golay-24

The proof test covers `decode_golay24`:

- zero codeword: `decode_golay24(&0)` returns 0 and leaves 0;
- all-ones codeword: returns 0 and leaves `0xFFFFFF`;
- one flipped bit: returns 1 and restores 0;
- two flipped bits: returns 2 and restores 0;
- an uncorrectable word: returns -1 and does not claim a correction.

This is a real decoder with deterministic vectors, it needs no fixtures, and it proves
that the harness compiles C sources, links them with the test, and reports failures.

### 5. Layout

```
tests/unit/
  CMakeLists.txt
  README.md
  third_party/doctest.h
  test_golay24.cpp
  build/            # generated, ignored
```

The README documents:

```
cmake -S tests/unit -B tests/unit/build
cmake --build tests/unit/build
ctest --test-dir tests/unit/build --output-on-failure
```

### 6. Isolation

The build directory lives under `tests/unit/build/` and is added to `.gitignore`. The
suite reads source files but never writes to `tinyGS/` or `.pio/`, so a subsequent release
build is byte-for-byte unaffected.

## Risks / Trade-offs

- [Vendored header grows the repository] → One 280 KB file, pinned and self-contained; no
  build-time network or package manager.
- [Only pure sources can be tested] → That is the point: the harness fails loudly if an
  Arduino-dependent file is added; the extraction of a pure core is a later, focused change.
- [Tests can drift from the firmware copy] → The test target compiles the same source file
  from `tinyGS/src`, not a copy.
- [CMake is a second build system] → It is test-only, does not touch PlatformIO, and is
  standard on Linux/macOS/Windows.
- [The smoke test is small] → It proves the pipeline; coverage grows in later changes with
  AX.25/AX.100/RS/PN9 vectors.

## Migration Plan

Additive: new files under `tests/unit/` and one `.gitignore` entry. No firmware, release
environment or generated artifact changes. Rollback is deleting `tests/unit/` and the
ignore entry.
