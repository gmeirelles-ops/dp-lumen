#!/usr/bin/env python3
"""Try tremo_loader SYNC repeatedly — use while holding BOOT and pulsing RST."""
from __future__ import annotations

import argparse
import struct
import sys
import time
import zlib

import serial


def sync_packet(cmd: int = 1, data: bytes = b"") -> bytes:
    pkt = struct.pack("<BBH", 0xFE, cmd, len(data)) + data
    return pkt + struct.pack("<IB", zlib.crc32(pkt) & 0xFFFFFFFF, 0xEF)


def try_sync(ser: serial.Serial, read_ms: float = 600) -> tuple[bool, str]:
    """Send SYNC and hunt for 0xFE response (tolerates garbage before header)."""
    ser.reset_output_buffer()
    ser.write(sync_packet())
    ser.flush()

    buf = bytearray()
    deadline = time.time() + (read_ms / 1000.0)
    while time.time() < deadline:
        waiting = ser.in_waiting
        chunk = ser.read(waiting if waiting else 1)
        if chunk:
            buf.extend(chunk)
        else:
            time.sleep(0.01)

    if not buf:
        return False, "no bytes"

    if b"\xfe" not in buf:
        return False, f"no 0xFE in {len(buf)} B: {bytes(buf[:16]).hex()}"

    idx = buf.find(b"\xfe")
    if len(buf) < idx + 4:
        return False, f"partial 0xFE only ({len(buf) - idx} B after marker)"

    header = bytes(buf[idx : idx + 4])
    start, status, rsp_len = struct.unpack("<BBH", header)
    if start != 0xFE:
        return False, f"bad start {header.hex()}"

    need = 4 + rsp_len + 5
    have = len(buf) - idx
    if have < need:
        extra = ser.read(need - have)
        buf.extend(extra)
        have = len(buf) - idx

    if have < need:
        return False, f"short packet need {need} got {have} (boot glimpsed)"

    pkt = bytes(buf[idx : idx + need])
    crc, end = struct.unpack("<IB", pkt[4 + rsp_len :])
    if end != 0xEF:
        return False, f"bad end 0x{end:02x}"
    if crc != (zlib.crc32(pkt[: 4 + rsp_len]) & 0xFFFFFFFF):
        return False, "crc error"

    return True, f"ok status={status}"


def main() -> int:
    parser = argparse.ArgumentParser(description="RA-08H bootloader SYNC test")
    parser.add_argument("-p", "--port", default="/dev/ttyUSB0")
    parser.add_argument("-t", "--seconds", type=float, default=60.0)
    args = parser.parse_args()

    print(f"Port {args.port} @ 115200 — SYNC hunt for {args.seconds:.0f}s")
    print("Jumper J2-7 -> J2-1 (+3V3) FIXO | pulse J2-6 -> GND durante o teste")
    print("TX conversor -> J2-8 | RX -> J2-9 | GND -> J2-3/4/5")
    print(">>> Pulse RST a cada 1-2 s enquanto roda <<<")
    print("-" * 60)

    ser = serial.Serial(args.port, 115200, timeout=0.05)
    deadline = time.time() + args.seconds
    attempt = 0
    last_msg = ""
    fe_hits = 0

    while time.time() < deadline:
        attempt += 1
        # keep a little RX backlog — do not flush before sync
        ok, msg = try_sync(ser)
        if ok:
            print(f"SUCCESS attempt {attempt}: {msg}")
            ser.close()
            print("Run: ./scripts/flash-dp-lumen-manual.sh")
            return 0

        if "0xFE" in msg or "partial 0xFE" in msg or "boot glimpsed" in msg:
            fe_hits += 1
            print(f"  {attempt:3d}: NEAR — {msg}")
        elif attempt <= 5 or attempt % 10 == 0:
            print(f"  {attempt:3d}: {msg}")

        last_msg = msg
        time.sleep(0.12)

    ser.close()
    print("-" * 60)
    print(f"FAILED after {attempt} attempts (saw 0xFE hints: {fe_hits})")
    print(f"Last: {last_msg}")
    if fe_hits:
        print("Bootloader respondeu parcialmente — firme RX no pin 9, repita RST mais vezes")
    else:
        print("Sem 0xFE — confira BOOT 7->1, pulse RST, TX pin 8")
    return 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except serial.SerialException as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
