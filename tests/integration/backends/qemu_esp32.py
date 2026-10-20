"""Classic ESP32 backend using the pinned QEMU fork with `esp32_wifi`."""
from __future__ import annotations

import os
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

import serial

from .base import BackendError, TargetBackend, free_port, wait_for_port

REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_IMAGE = "tinygs-qemu-esp32"


class QemuEsp32Backend(TargetBackend):
    """Run the classic ESP32 firmware in QEMU behind the common target interface."""

    name = "esp32-qemu"

    def __init__(
        self,
        artifact_dir: Path,
        *,
        firmware: Path | None = None,
        qemu_bin: str | None = None,
        docker_image: str = DEFAULT_IMAGE,
        wifi: bool = True,
        web: bool = True,
    ) -> None:
        super().__init__(artifact_dir)
        self.firmware = Path(firmware) if firmware else self._resolve_firmware()
        self.wifi_ssid = os.environ.get("TINYGS_QEMU_WIFI_SSID", "Open Wifi")
        self.wifi_password = os.environ.get("TINYGS_QEMU_WIFI_PASS", "")
        self.qemu_bin = qemu_bin or os.environ.get("TINYGS_QEMU_BIN")
        self.docker_image = docker_image or os.environ.get(
            "TINYGS_QEMU_IMAGE", DEFAULT_IMAGE
        )
        self.wifi = wifi
        self.web = web
        self.uart_port = 0
        self.monitor_port = 0
        self.web_port = 0
        self._process: subprocess.Popen[bytes] | None = None
        self._log_file = None
        capabilities = {"serial", "reset", "flash", "improv"}
        self.capabilities = frozenset(capabilities)

    def _resolve_firmware(self) -> Path:
        explicit = os.environ.get("TINYGS_ESP32_IMAGE")
        candidates = [
            Path(explicit) if explicit else None,
            REPO_ROOT / "tests" / "emu-esp32" / "firmware-full.bin",
            REPO_ROOT / ".pio" / "build" / "ESP32" / "firmware-full.bin",
            REPO_ROOT / ".pio" / "build" / "ESP32" / "firmware.factory.bin",
        ]
        for candidate in candidates:
            if candidate is not None and candidate.is_file():
                return candidate
        # Try to produce the merged image from the PlatformIO build outputs.
        script = REPO_ROOT / "tests" / "emu-esp32" / "make_image.py"
        if script.is_file():
            result = subprocess.run(
                [sys.executable, str(script)],
                cwd=REPO_ROOT,
                capture_output=True,
                text=True,
            )
            if result.returncode == 0:
                output = REPO_ROOT / "tests" / "emu-esp32" / "firmware-full.bin"
                if output.is_file():
                    return output
        raise BackendError(
            "no merged ESP32 image found: run `pio run -e ESP32`, then "
            "`python tests/emu-esp32/make_image.py`, or set TINYGS_ESP32_IMAGE"
        )

    def start(self) -> None:
        flash = self.artifact_dir / "flash.bin"
        shutil.copyfile(self.firmware, flash)
        self.uart_port = free_port()
        self.monitor_port = free_port()
        nic = "user,model=esp32_wifi"
        if self.web:
            self.web_port = free_port()
            nic += f",hostfwd=tcp:127.0.0.1:{self.web_port}-:80"
        drive_file = str(flash) if self.qemu_bin else "/flash.bin"
        qemu_args = [
            "-machine",
            "esp32",
            "-nographic",
            "-drive",
            f"file={drive_file},if=mtd,format=raw",
            "-serial",
            f"tcp:127.0.0.1:{self.uart_port},server=on,wait=off",
            "-monitor",
            f"tcp:127.0.0.1:{self.monitor_port},server=on,wait=off",
            "-global",
            "driver=timer.esp32.timg,property=wdt_disable,value=true",
        ]
        if self.wifi:
            qemu_args += ["-nic", nic]
        if self.qemu_bin:
            cmd = [self.qemu_bin, *qemu_args]
        else:
            cmd = [
                "docker",
                "run",
                "--rm",
                "--network",
                "host",
                "-v",
                f"{flash}:/flash.bin:rw",
                self.docker_image,
                *qemu_args,
            ]
        self._log_file = open(self.log_path, "wb")
        self._process = subprocess.Popen(
            cmd,
            stdout=self._log_file,
            stderr=subprocess.STDOUT,
            stdin=subprocess.DEVNULL,
        )
        try:
            wait_for_port("127.0.0.1", self.uart_port, timeout=60)
            wait_for_port("127.0.0.1", self.monitor_port, timeout=60)
        except BackendError as exc:
            self.stop()
            raise BackendError(f"{exc}\ncommand: {' '.join(cmd)}") from exc

    def stop(self) -> None:
        if self._process is not None:
            try:
                with socket.create_connection(
                    ("127.0.0.1", self.monitor_port), timeout=2
                ) as sock:
                    sock.sendall(b"quit\n")
            except OSError:
                pass
            try:
                self._process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self._process.terminate()
                try:
                    self._process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    self._process.kill()
                    self._process.wait(timeout=5)
            self._process = None
        if self._log_file is not None:
            self._log_file.close()
            self._log_file = None

    def reset(self) -> None:
        with socket.create_connection(
            ("127.0.0.1", self.monitor_port), timeout=5
        ) as sock:
            sock.sendall(b"system_reset\n")
            time.sleep(0.3)

    def open_serial(self, timeout: float = 0.2) -> serial.SerialBase:
        return serial.serial_for_url(
            f"socket://127.0.0.1:{self.uart_port}",
            baudrate=115200,
            timeout=timeout,
        )

    @property
    def web_url(self) -> str | None:
        return f"http://127.0.0.1:{self.web_port}/" if self.web else None
