#!/usr/bin/env python3
"""Build the merged 4 MB flash image used by the classic ESP32 QEMU backend."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
BUILD_DIR = REPO_ROOT / ".pio" / "build" / "ESP32"
OUTPUT = Path(__file__).resolve().parent / "firmware-full.bin"


def find_boot_app0() -> Path:
    core = Path.home() / ".platformio" / "packages"
    matches = sorted(core.glob("*/tools/partitions/boot_app0.bin"))
    if not matches:
        raise SystemExit(
            f"boot_app0.bin not found under {core}; install the espressif32 platform first"
        )
    return matches[0]


def main() -> int:
    required = [
        BUILD_DIR / "bootloader.bin",
        BUILD_DIR / "partitions.bin",
        BUILD_DIR / "firmware.bin",
    ]
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise SystemExit(
            "missing ESP32 build outputs:\n  "
            + "\n  ".join(missing)
            + f"\nrun `pio run -e ESP32` first"
        )
    boot_app0 = find_boot_app0()
    cmd = [
        sys.executable,
        "-m",
        "esptool",
        "--chip",
        "esp32",
        "merge_bin",
        "-o",
        str(OUTPUT),
        "--fill-flash-size",
        "4MB",
        "--flash_mode",
        "dio",
        "--flash_freq",
        "40m",
        "--flash_size",
        "4MB",
        "0x1000",
        str(required[0]),
        "0x8000",
        str(required[1]),
        "0xe000",
        str(boot_app0),
        "0x10000",
        str(required[2]),
    ]
    print("+", " ".join(cmd))
    result = subprocess.run(cmd)
    if result.returncode != 0:
        return result.returncode
    print(f"wrote {OUTPUT} ({OUTPUT.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
