"""esp-emulator backend (ESP32-S3 / ESP32-C3)."""
from __future__ import annotations

import os
import shutil
import socket
import subprocess
import time
from pathlib import Path

import serial

from .base import (
    BackendError,
    TargetBackend,
    free_port,
    wait_for_port,
)

REPO_ROOT = Path(__file__).resolve().parents[3]

ENV_BY_TARGET = {
    "esp32s3-emu": ("ESP32-S3", "esp32s3"),
    "esp32c3-emu": ("ESP32-C3", "esp32c3"),
}

#: Test-only PlatformIO environment with the radio bypass, used when present.
TEST_ENV_BY_TARGET = {
    "esp32s3-emu": "ESP32-S3-emu",
    "esp32c3-emu": "ESP32-C3-emu",
}


class EspEmuBackend(TargetBackend):
    """Run the firmware in esp-emulator and expose its UART/control channels."""

    def __init__(
        self,
        name: str,
        artifact_dir: Path,
        *,
        esp_emu_bin: str | None = None,
        firmware: Path | None = None,
        wifi_ssid: str = "tinygs-test",
        wifi_password: str = "testpassword",
        web: bool = False,
    ) -> None:
        super().__init__(artifact_dir)
        if name not in ENV_BY_TARGET:
            raise BackendError(f"unsupported esp-emulator target: {name}")
        self.name = name
        self.env, self.chip = ENV_BY_TARGET[name]
        self.esp_emu_bin = (
            esp_emu_bin
            or os.environ.get("TINYGS_ESP_EMU")
            or shutil.which("esp-emu")
        )
        self.firmware = Path(firmware) if firmware else self._resolve_firmware()
        self.wifi_ssid = wifi_ssid
        self.wifi_password = wifi_password
        self.web = web
        self.uart_port = 0
        self.control_port = 0
        self.web_port = 0
        self._process: subprocess.Popen[bytes] | None = None
        self._log_file = None
        self.capabilities = frozenset({"serial", "reset", "improv"})

    def _resolve_firmware(self) -> Path:
        explicit = os.environ.get("TINYGS_FIRMWARE")
        if explicit:
            path = Path(explicit)
            if not path.is_file():
                raise BackendError(f"TINYGS_FIRMWARE does not exist: {path}")
            return path
        candidate = REPO_ROOT / ".pio" / "build" / self.env / "firmware.factory.bin"
        if candidate.is_file():
            return candidate
        raise BackendError(
            f"no merged image for {self.env}: expected {candidate}. "
            f"Run `pio run -e {self.env}` or set TINYGS_FIRMWARE."
        )

    def start(self) -> None:
        if not self.esp_emu_bin:
            raise BackendError(
                "esp-emu was not found: install it or set TINYGS_ESP_EMU"
            )
        self.uart_port = free_port()
        self.control_port = free_port()
        net = "user"
        if self.web:
            self.web_port = free_port()
            net = f"user,hostfwd=tcp:127.0.0.1:{self.web_port}-:80"
        cmd = [
            self.esp_emu_bin,
            "--chip",
            self.chip,
            "--firmware",
            str(self.firmware),
            "--net",
            net,
            "--wifi-ssid",
            self.wifi_ssid,
            "--wifi-password",
            self.wifi_password,
            "--uart-tcp",
            f"127.0.0.1:{self.uart_port}",
            "--control-tcp",
            f"127.0.0.1:{self.control_port}",
        ]
        self._log_file = open(self.log_path, "wb")
        self._process = subprocess.Popen(
            cmd,
            stdout=self._log_file,
            stderr=subprocess.STDOUT,
            stdin=subprocess.DEVNULL,
        )
        try:
            wait_for_port("127.0.0.1", self.uart_port, timeout=30)
            wait_for_port("127.0.0.1", self.control_port, timeout=30)
        except BackendError as exc:
            self.stop()
            raise BackendError(f"{exc}\ncommand: {' '.join(cmd)}") from exc

    def stop(self) -> None:
        if self._process is not None:
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
            ("127.0.0.1", self.control_port), timeout=5
        ) as sock:
            sock.sendall(b"reset\n")
            reply = sock.recv(64)
        if b"ok" not in reply:
            raise BackendError(f"esp-emu reset failed: {reply!r}")

    def open_serial(self, timeout: float = 0.2) -> serial.SerialBase:
        return serial.serial_for_url(
            f"socket://127.0.0.1:{self.uart_port}",
            baudrate=115200,
            timeout=timeout,
        )

    @property
    def web_url(self) -> str | None:
        return f"http://127.0.0.1:{self.web_port}/" if self.web else None
