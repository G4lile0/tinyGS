"""Shared pytest setup for the backend-neutral integration suite."""
from __future__ import annotations

import os
import re
import sys
import time
from pathlib import Path

import pytest

INTEGRATION_ROOT = Path(__file__).resolve().parent
if str(INTEGRATION_ROOT) not in sys.path:
    sys.path.insert(0, str(INTEGRATION_ROOT))

from backends.base import BackendError, TargetBackend  # noqa: E402
from backends.esp_emu import EspEmuBackend  # noqa: E402
from backends.qemu_esp32 import QemuEsp32Backend  # noqa: E402

TARGETS = ("esp32s3-emu", "esp32c3-emu", "esp32-qemu")

#: Static capability matrix used to skip tests before a backend is started.
TARGET_CAPABILITIES: dict[str, frozenset[str]] = {
    # Radio init blocks a configured boot until a test-only bypass is approved;
    # WiFi/MQTT/web tests are therefore skipped. See tests/emu-esp32/SPIKE.md.
    "esp32s3-emu": frozenset({"serial", "reset", "improv"}),
    "esp32c3-emu": frozenset({"serial", "reset", "improv"}),
    "esp32-qemu": frozenset({"serial", "reset", "flash", "improv"}),
}


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption(
        "--target",
        action="store",
        default=os.environ.get("TINYGS_TARGET", "esp32s3-emu"),
        help=f"integration target ({', '.join(TARGETS)})",
    )


def pytest_configure(config: pytest.Config) -> None:
    config.addinivalue_line(
        "markers",
        "requires(*capabilities): test needs backend capabilities "
        "(serial, reset, flash, improv, wifi, web, mqtt)",
    )


def _selected_target(config: pytest.Config) -> str:
    name = config.getoption("--target")
    if name not in TARGET_CAPABILITIES:
        raise pytest.UsageError(
            f"unknown --target {name!r}; supported: {', '.join(TARGETS)}"
        )
    return name


def pytest_collection_modifyitems(
    config: pytest.Config, items: list[pytest.Item]
) -> None:
    name = _selected_target(config)
    capabilities = TARGET_CAPABILITIES[name]
    for item in items:
        marker = item.get_closest_marker("requires")
        if marker is None:
            continue
        missing = set(marker.args) - capabilities
        if missing:
            item.add_marker(
                pytest.mark.skip(
                    reason=(
                        f"target {name} does not provide "
                        f"{', '.join(sorted(missing))}"
                    )
                )
            )


def _artifact_dir(node_name: str) -> Path:
    stamp = time.strftime("%Y%m%d-%H%M%S")
    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", node_name)
    path = INTEGRATION_ROOT / "artifacts" / stamp / safe
    path.mkdir(parents=True, exist_ok=True)
    return path


def _make_backend(name: str, artifact_dir: Path) -> TargetBackend:
    if name in ("esp32s3-emu", "esp32c3-emu"):
        return EspEmuBackend(name, artifact_dir, web=True)
    if name == "esp32-qemu":
        return QemuEsp32Backend(artifact_dir)
    raise BackendError(f"unsupported target: {name}")


@pytest.fixture
def target(request: pytest.FixtureRequest) -> TargetBackend:
    name = _selected_target(request.config)
    backend = _make_backend(name, _artifact_dir(request.node.name))
    try:
        backend.start()
    except BackendError as exc:
        (backend.artifact_dir / "startup-error.txt").write_text(str(exc))
        raise
    try:
        yield backend
    finally:
        backend.stop()
