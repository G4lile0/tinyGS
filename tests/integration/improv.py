"""Minimal Improv-serial v1 client used by the shared integration tests."""
from __future__ import annotations

IMPROV_MARKER = b"IMPROV"
IMPROV_VERSION = 1

TYPE_RPC = 0x03
CMD_WIFI_SETTINGS = 0x01


def _checksum(data: bytes) -> int:
    return sum(data) & 0xFF


def frame_rpc(command: int, frame_data: bytes = b"") -> bytes:
    payload = bytes([command, len(frame_data)]) + frame_data
    packet = IMPROV_MARKER + bytes([IMPROV_VERSION, TYPE_RPC, len(payload)]) + payload
    return packet + bytes([_checksum(packet)])


def frame_wifi_settings(ssid: str, password: str) -> bytes:
    ssid_b = ssid.encode("utf-8")
    pass_b = password.encode("utf-8")
    if len(ssid_b) > 255 or len(pass_b) > 255:
        raise ValueError("Improv SSID/password must be at most 255 bytes")
    data = bytes([len(ssid_b)]) + ssid_b + bytes([len(pass_b)]) + pass_b
    return frame_rpc(CMD_WIFI_SETTINGS, data)
