"""Contract checks for the backend interface, without starting a target."""
from __future__ import annotations

from pathlib import Path

from backends.base import TargetBackend, read_until


class FakeSerial:
    def __init__(self, chunks: list[bytes]) -> None:
        self._chunks = list(chunks)

    def read(self, size: int) -> bytes:
        return self._chunks.pop(0) if self._chunks else b""


class FakeBackend(TargetBackend):
    name = "fake"
    capabilities = frozenset({"serial", "reset"})

    def __init__(self, artifact_dir: Path) -> None:
        super().__init__(artifact_dir)
        self.started = False

    def start(self) -> None:
        self.started = True

    def stop(self) -> None:
        self.started = False

    def reset(self) -> None:
        pass

    def open_serial(self, timeout: float = 0.2) -> FakeSerial:
        return FakeSerial([b"boot\n", b"TinyGS Version 1\n"])


def test_backend_contract(tmp_path):
    backend = FakeBackend(tmp_path)
    backend.start()
    assert backend.started
    assert backend.supports("serial")
    assert not backend.supports("web")
    assert backend.web_url is None

    port = backend.open_serial()
    output = read_until(port, b"TinyGS Version", timeout=1)
    assert b"TinyGS Version" in output

    backend.stop()
    assert not backend.started
