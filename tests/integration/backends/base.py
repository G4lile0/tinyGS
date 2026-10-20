"""Backend-neutral contract for integration targets.

The integration tests must not know whether the firmware runs in esp-emulator or in
QEMU. They talk to a `TargetBackend`: start it, read/write its serial console, reset
it, and ask which capabilities it provides.
"""
from __future__ import annotations

import socket
import time
from abc import ABC, abstractmethod
from pathlib import Path

import serial

#: Capabilities a backend can declare. Tests that need one use
#: `@pytest.mark.requires("web")` and are skipped when it is absent.
CAPABILITIES = frozenset(
    {"serial", "reset", "flash", "improv", "wifi", "web", "mqtt"}
)


class BackendError(RuntimeError):
    """The target could not be started, reset or reached."""


class TargetBackend(ABC):
    """Common interface implemented by every platform backend."""

    name: str = "?"
    capabilities: frozenset[str] = frozenset()
    #: Credentials the backend expects the firmware to use after provisioning.
    wifi_ssid: str | None = None
    wifi_password: str | None = None

    def __init__(self, artifact_dir: Path) -> None:
        self.artifact_dir = Path(artifact_dir).resolve()
        self.artifact_dir.mkdir(parents=True, exist_ok=True)

    @property
    def log_path(self) -> Path:
        return self.artifact_dir / "backend.log"

    def supports(self, capability: str) -> bool:
        return capability in self.capabilities

    @abstractmethod
    def start(self) -> None:
        """Start the emulator and wait until its serial endpoint is reachable."""

    @abstractmethod
    def stop(self) -> None:
        """Stop the emulator and release its resources."""

    @abstractmethod
    def reset(self) -> None:
        """Reset the guest without restarting the emulator process."""

    @abstractmethod
    def open_serial(self, timeout: float = 0.2) -> serial.SerialBase:
        """Open the guest console as a pyserial stream."""

    @property
    def web_url(self) -> str | None:
        """Host URL for the device web console, or None when unavailable."""
        return None


def free_port() -> int:
    """Reserve a free loopback TCP port for a backend endpoint."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return int(sock.getsockname()[1])


def wait_for_port(host: str, port: int, timeout: float = 30.0) -> None:
    """Wait until a TCP listener accepts connections."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.2):
                return
        except OSError:
            time.sleep(0.05)
    raise BackendError(
        f"timed out after {timeout:.0f}s waiting for {host}:{port}; "
        "check the backend log"
    )


def read_until(
    port: serial.SerialBase, needle: bytes, timeout: float = 30.0
) -> bytes:
    """Read until `needle` appears, or until the timeout expires."""
    deadline = time.monotonic() + timeout
    data = bytearray()
    while time.monotonic() < deadline:
        chunk = port.read(4096)
        if chunk:
            data.extend(chunk)
            if needle in data:
                return bytes(data)
        else:
            time.sleep(0.02)
    return bytes(data)


def read_for(port: serial.SerialBase, seconds: float) -> bytes:
    """Read everything the console emits during `seconds`."""
    deadline = time.monotonic() + seconds
    data = bytearray()
    while time.monotonic() < deadline:
        chunk = port.read(4096)
        if chunk:
            data.extend(chunk)
        else:
            time.sleep(0.02)
    return bytes(data)
