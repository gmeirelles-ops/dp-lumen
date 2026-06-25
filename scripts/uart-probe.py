#!/usr/bin/env python3
"""Listen on UART and print raw bytes (hex + ASCII) — diagnose J2 wiring."""
import argparse
import sys
import time

import serial


def main() -> int:
    parser = argparse.ArgumentParser(description="UART probe for RA-08H J2")
    parser.add_argument("-p", "--port", default="/dev/ttyUSB0")
    parser.add_argument("-b", "--baud", type=int, default=115200)
    parser.add_argument("-t", "--seconds", type=float, default=10.0)
    args = parser.parse_args()

    print(f"Listening {args.port} @ {args.baud} for {args.seconds}s")
    print("Pulse RST (J2-6->GND) or reset board now...")
    print("-" * 50)

    with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
        ser.reset_input_buffer()
        end = time.time() + args.seconds
        total = 0
        data_accum = bytearray()
        while time.time() < end:
            chunk = ser.read(256)
            if chunk:
                total += len(chunk)
                data_accum.extend(chunk)
                print(f"[{len(chunk)}] hex: {chunk.hex()}")
                ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
                print(f"      ascii: {ascii_part}")

    print("-" * 50)
    if total == 0:
        print("NO DATA — check TX->J2-8, RX->J2-9, GND, board power, 3V3 logic")
    else:
        print(f"Total {total} bytes received")
        if b"\xfe" in data_accum:
            print("Hint: 0xFE seen — close to bootloader framing; try flash with BOOT held")
        elif all(b == 0 for b in data_accum):
            print("Hint: mostly zeros — RX may be floating or firmware not transmitting")
        else:
            print("Hint: garbage/noise — likely NOT in boot mode (AT app or bad wiring)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except serial.SerialException as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
