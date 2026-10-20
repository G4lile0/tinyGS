#!/usr/bin/env python3
"""Reproduce the QEMU WiFi path and capture the firmware console + QEMU log.

Usage:
    tests/integration/.venv/bin/python tests/emu-esp32/reproduce_wifi.py

Artifacts:
    tests/emu-esp32/artifacts/wifi-repro/serial.log
    tests/emu-esp32/artifacts/wifi-repro/backend.log
"""
from __future__ import annotations

import json
import sys
import time
import urllib.request
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "tests" / "integration"))

from backends.base import read_until  # noqa: E402
from backends.qemu_esp32 import QemuEsp32Backend  # noqa: E402
from improv import frame_wifi_settings  # noqa: E402

OUT = REPO_ROOT / "tests" / "integration" / "artifacts" / "wifi-repro"


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    serial_log = OUT / "serial.log"
    backend = QemuEsp32Backend(OUT, wifi=True, web=True)
    print(f"firmware: {backend.firmware}")
    print(f"log: {backend.log_path}")
    summary: dict[str, object] = {}
    serial_log.write_bytes(b"")
    backend.start()
    try:
        port = backend.open_serial(timeout=0.2)
    except Exception as exc:
        backend.stop()
        raise SystemExit(f"could not open serial: {exc}") from exc

    def capture(seconds: float) -> bytes:
        deadline = time.monotonic() + seconds
        data = bytearray()
        while time.monotonic() < deadline:
            try:
                chunk = port.read(4096)
            except Exception:
                break
            if chunk:
                data.extend(chunk)
                with serial_log.open("ab") as handle:
                    handle.write(chunk)
            else:
                time.sleep(0.02)
        return bytes(data)

    boot = capture(30)
    print("boot captured:", len(boot), "bytes; controls:", b"Controls" in boot)
    print("sending Improv WiFi settings: SSID=%r" % backend.wifi_ssid)
    port.write(
        frame_wifi_settings(backend.wifi_ssid or "", backend.wifi_password or "")
    )
    port.flush()

    after = capture(60)
    text = after.decode("utf-8", "replace")
    for needle in (
        "TinyGS Version",
        "Controls",
        "Initializing",
        "emulator build",
        "Connected to MQTT",
        "Attempting MQTT",
        "IP",
        "Guru Meditation",
    ):
        summary[needle] = needle in text

    web_status = None
    if backend.web_url:
        try:
            with urllib.request.urlopen(backend.web_url, timeout=3) as response:
                body = response.read(4000).decode("utf-8", "replace")
            web_status = f"HTTP {response.status}, TinyGS={('TinyGS' in body)}"
        except Exception as exc:
            web_status = repr(exc)
    summary["web"] = web_status
    summary["serial_bytes_after_provisioning"] = len(after)
    summary["qemu_log_bytes"] = (
        backend.log_path.stat().st_size if backend.log_path.exists() else 0
    )
    (OUT / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps(summary, indent=2))
    try:
        port.close()
    except Exception:
        pass
    backend.stop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
