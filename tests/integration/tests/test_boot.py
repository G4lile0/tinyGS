import re

import pytest

from backends.base import read_until


@pytest.mark.requires("serial")
def test_boot_reaches_tinygs_console(target):
    with target.open_serial(timeout=0.2) as port:
        output = read_until(port, b"TinyGS Version", timeout=120)
        output += read_until(
            port, b"------------- Controls -------------", timeout=60
        )

    assert b"TinyGS Version" in output, output[-2000:]
    assert re.search(rb"Chip\s+ESP32", output), output[-2000:]
    assert re.search(rb"Generated OTP: \d{6}", output), output[-2000:]
    assert b"------------- Controls -------------" in output, output[-2000:]
    assert b"Guru Meditation" not in output
    assert b"abort()" not in output


@pytest.mark.requires("serial")
def test_console_command_is_answered(target):
    with target.open_serial(timeout=0.2) as port:
        read_until(port, b"------------- Controls -------------", timeout=120)
        port.write(b"!o\n")
        port.flush()
        output = read_until(port, b"OTP Code:", timeout=15)

    assert b"OTP Code:" in output, output[-2000:]


@pytest.mark.requires("reset")
def test_reset_boots_the_same_firmware_again(target):
    with target.open_serial(timeout=0.2) as port:
        assert b"TinyGS Version" in read_until(
            port, b"TinyGS Version", timeout=120
        )

    target.reset()

    with target.open_serial(timeout=0.2) as port:
        output = read_until(port, b"TinyGS Version", timeout=120)

    assert b"TinyGS Version" in output, output[-2000:]
