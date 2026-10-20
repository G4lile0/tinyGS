"""Shared WiFi provisioning test: Improv over serial, then the device web."""
from __future__ import annotations

import time
import urllib.request

import pytest
import serial

from backends.base import read_for, read_until
from improv import frame_wifi_settings


@pytest.mark.requires("improv", "wifi", "web")
def test_improv_provisioning_exposes_device_web(target):
    assert target.wifi_ssid is not None
    with target.open_serial(timeout=0.2) as port:
        read_until(port, b"------------- Controls -------------", timeout=120)
        port.write(frame_wifi_settings(target.wifi_ssid, target.wifi_password or ""))
        port.flush()
        try:
            read_for(port, 5.0)
        except serial.SerialException:
            # The guest reboots after provisioning; the console may disappear.
            pass

    deadline = time.monotonic() + 90
    last_error: str | None = None
    while time.monotonic() < deadline:
        try:
            with urllib.request.urlopen(target.web_url, timeout=3) as response:
                body = response.read(4000).decode("utf-8", "replace")
            if "TinyGS" in body:
                return
            last_error = f"HTTP {response.status}, no TinyGS title"
        except Exception as exc:  # connection refused/reset while the guest boots
            last_error = repr(exc)
        time.sleep(2)
    raise AssertionError(
        f"device web did not come up at {target.web_url} after provisioning: "
        f"{last_error}"
    )
