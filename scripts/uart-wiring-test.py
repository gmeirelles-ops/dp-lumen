#!/usr/bin/env python3
"""Find working UART wiring for RA-08H on dp-lumen J2."""
from __future__ import annotations

import argparse
import struct
import sys
import time
import zlib

import serial

SYNC_PKT = (
    struct.pack("<BBH", 0xFE, 1, 0)
    + struct.pack("<IB", zlib.crc32(struct.pack("<BBH", 0xFE, 1, 0)) & 0xFFFFFFFF, 0xEF)
)


def classify(data: bytes) -> str:
    if not data:
        return "silent"
    if data[0:1] == b"\xfe":
        return "bootloader"
    text = data.decode("ascii", errors="ignore")
    if "OK" in text or text.strip().startswith("+"):
        return "at_ok"
    if len(set(data[: min(32, len(data))])) <= 4:
        return "garbage_repetitive"
    return "garbage"


def probe(port: str, baud: int, payload: bytes, label: str) -> str:
    with serial.Serial(port, baud, timeout=0.5) as ser:
        ser.reset_input_buffer()
        ser.write(payload)
        time.sleep(0.25)
        data = ser.read(256)
    kind = classify(data)
    if kind == "silent":
        print(f"  {label:12s} @ {baud:6d}: no response")
        return kind
    ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in data[:48])
    print(
        f"  {label:12s} @ {baud:6d}: {len(data):3d} B  "
        f"[{kind}]  hex={data[:16].hex()}  ascii={ascii_part!r}"
    )
    if kind == "bootloader":
        print("    -> tremo_loader response — boot mode OK, run flash-dp-lumen-manual.sh")
    return kind


def main() -> int:
    parser = argparse.ArgumentParser(description="RA-08H UART wiring helper")
    parser.add_argument("-p", "--port", default="/dev/ttyUSB0")
    args = parser.parse_args()

    print("dp-lumen J2 — RA-08H UART map")
    print("  Flash (boot):  PC TX -> pin 8 (IO16), PC RX -> pin 9 (IO17)")
    print("  AT (runtime):  PC TX -> pin 10 (IO60), PC RX -> pin 9 (IO17)")
    print("  Common mistake: TX on pin 10 works for AT but NEVER for flash")
    print()
    print(f"Probing {args.port} (placa ligada, sem boot manual)...")
    print("-" * 72)

    results = []
    for baud in (115200, 9600):
        results.append(("AT", baud, probe(args.port, baud, b"AT\r\n", "AT")))
        results.append(("SYNC", baud, probe(args.port, baud, SYNC_PKT, "SYNC")))

    print("-" * 72)
    at115 = next((k for l, b, k in results if l == "AT" and b == 115200), "silent")
    sync115 = next((k for l, b, k in results if l == "SYNC" and b == 115200), "silent")

    print("Interpretation:")
    if sync115 == "bootloader":
        print("  Bootloader active — run ./scripts/flash-dp-lumen-manual.sh")
    elif at115 in ("garbage_repetitive", "garbage") and sync115 != "silent":
        print("  RX hears noise/garbage — check GND, RX on pin 9, 3V3 logic, TX pin")
    elif at115 in ("garbage_repetitive", "garbage"):
        print("  Garbage on RX — TX likely wrong pin or baud; try TX on pin 10 for AT test")
    elif at115 == "at_ok":
        print("  AT OK on current wiring — TX is on pin 10 (runtime). Move TX to pin 8 for flash")
    elif at115 == "silent" and sync115 == "silent":
        print("  Silent in normal mode — EXPECTED if TX is on pin 8 (flash UART)")
        print("  Next: jumper pin 7->1, pulse pin 6->GND, run boot-sync-test.py")
    else:
        print("  Partial/unclear — confirm TX pin 8, RX pin 9, GND, placa ligada")
    print()
    print("Boot manual: jumper pin 7->1 (+3V3), pulse pin 6->GND, then boot-sync-test.py")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except serial.SerialException as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
