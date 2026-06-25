#!/usr/bin/env python3
"""Apply dp-lumen patches to Ai-Thinker tremo_loader.py (idempotent)."""
from __future__ import annotations

import pathlib
import re
import sys


def patch_wait_response(text: str) -> str:
    if "Wrong response packet (got" in text:
        return text
    text = text.replace(
        '            raise CmdException("Wrong response packet")\n'
        '        if rsp_data_len > 512:',
        '            raise CmdException("Wrong response packet (got %r)" % header)\n'
        '        if rsp_data_len > 512:',
        1,
    )
    if "Short response header" in text:
        return text
    old = (
        '        header = self.ser.read(4)\n'
        '        if header == b\'\':\n'
        '            raise CmdException("Read response header timeout")\n'
        '        (start, status, rsp_data_len) = struct.unpack(\'<BBH\', header)'
    )
    new = (
        '        header = self.ser.read(4)\n'
        '        if len(header) < 4:\n'
        '            if header == b\'\':\n'
        '                raise CmdException("Read response header timeout")\n'
        '            raise CmdException("Short response header (%d bytes): %r" % (len(header), header))\n'
        '        (start, status, rsp_data_len) = struct.unpack(\'<BBH\', header)'
    )
    if old not in text:
        raise RuntimeError("wait_response block not found")
    return text.replace(old, new, 1)


def patch_hw_reset(text: str) -> str:
    if "profile=None" in text:
        return text
    pattern = re.compile(
        r"    def hw_reset\(self, mode=0.*?\):.*?def connect\(",
        re.DOTALL,
    )
    replacement = '''    def hw_reset(self, mode=0, profile=None):
        if not mode:
            self.ser.setRTS(True)
            time.sleep(0.1)
            self.ser.setRTS(False)
            return

        if profile is None:
            profile = os.environ.get("TREMO_RESET_MODE", "dtr_rts")

        if profile == "dtr_cap":
            # FT232/CH340 6-pin: DTR -> J2-7 (BOOT); 100nF DTR -> J2-6 (RST), 10k RST -> 3V3
            self.ser.setDTR(False)
            time.sleep(0.05)
            self.ser.setDTR(True)
            time.sleep(0.05)
            self.ser.setDTR(False)
            time.sleep(0.05)
            self.ser.setDTR(True)
            time.sleep(0.35)
            return

        # DTR -> J2-7 (BOOT), RTS/CTS -> J2-6 (RST)
        self.ser.setDTR(False)
        self.ser.setRTS(False)
        time.sleep(0.05)
        self.ser.setDTR(True)
        time.sleep(0.05)
        self.ser.setRTS(True)
        time.sleep(0.15)
        self.ser.setRTS(False)
        time.sleep(0.35)

    def connect('''
    if not pattern.search(text):
        raise RuntimeError("hw_reset block not found")
    return pattern.sub(replacement, text, count=1)


def patch_drain_and_sync(text: str) -> str:
    if "def drain_input(self" in text:
        return text
    old = '''    def sync(self):
        self.requeset(self.CMD_SYNC)
        self.wait_response()

    def hw_reset('''
    new = '''    def drain_input(self, ms=150):
        end = time.time() + (ms / 1000.0)
        while time.time() < end:
            waiting = self.ser.in_waiting
            if waiting:
                self.ser.read(waiting)
            else:
                time.sleep(0.01)

    def sync(self, retries=3):
        last_error = None
        for _ in range(retries):
            try:
                self.drain_input(80)
                self.requeset(self.CMD_SYNC)
                self.wait_response()
                return
            except CmdException as exc:
                last_error = exc
                time.sleep(0.03)
        raise last_error

    def hw_reset('''
    if old not in text:
        raise RuntimeError("sync block not found")
    return text.replace(old, new, 1)


def patch_connect_retries(text: str) -> str:
    text = text.replace(
        "    def connect(self, retry=3):",
        "    def connect(self, retry=8):",
        1,
    )
    if "profile = profiles[attempt % len(profiles)]" in text:
        return text
    old = '''        for _ in range(retry):
            try:
                self.hw_reset(1)

                last_error = None'''
    new = '''        profiles = [os.environ.get("TREMO_RESET_MODE", "dtr_rts")]
        if profiles[0] == "auto":
            profiles = ["dtr_rts", "dtr_cap"]

        last_error = None
        for attempt in range(retry):
            profile = profiles[attempt % len(profiles)]
            try:
                self.hw_reset(1, profile=profile)

                last_error = None'''
    if old not in text:
        raise RuntimeError("connect retry loop not found")
    return text.replace(old, new, 1)


def patch_flash_initial_baud(text: str) -> str:
    if "TremoLoader(args.port, 115200)" in text:
        return text
    old = "    # flash\n    tremo = TremoLoader(args.port)\n    tremo.connect()"
    new = "    # flash\n    tremo = TremoLoader(args.port, 115200)\n    tremo.connect()"
    if old not in text:
        raise RuntimeError("tremo_flash initial baud block not found")
    return text.replace(old, new, 1)


def ensure_import_os(text: str) -> str:
    if re.search(r"^import os\b", text, re.MULTILINE):
        return text
    return text.replace("import serial", "import os\nimport serial", 1)


def main() -> int:
    path = pathlib.Path(sys.argv[1])
    text = path.read_text()
    text = ensure_import_os(text)
    text = patch_wait_response(text)
    text = patch_drain_and_sync(text)
    text = patch_hw_reset(text)
    text = patch_connect_retries(text)
    text = patch_flash_initial_baud(text)
    path.write_text(text)
    print(f"patched: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
