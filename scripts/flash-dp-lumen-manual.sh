#!/usr/bin/env bash
# Flash dp-lumen with manual BOOT/RST — retries SYNC for ~45s after Enter.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SDK_ROOT="${RA08H_SDK_PATH:-$REPO_ROOT/Ai-Thinker-LoRaWAN-Ra-08}"
PROJECT_DIR="${RA08H_PROJECT_DIR:-$REPO_ROOT/sdk/ra08h-dp-lumen}"
BIN="$PROJECT_DIR/out/ra08h-dp-lumen.bin"
SERIAL_PORT="${SERIAL_PORT:-/dev/ttyUSB0}"
SERIAL_BAUDRATE="${SERIAL_BAUDRATE:-921600}"
SYNC_SECONDS="${SYNC_SECONDS:-45}"

if [[ ! -e "$SERIAL_PORT" ]]; then
    echo "error: port $SERIAL_PORT not found" >&2
    exit 1
fi

if fuser "$SERIAL_PORT" >/dev/null 2>&1; then
    echo "error: $SERIAL_PORT busy — close serial monitor first" >&2
    exit 1
fi

if [[ ! -f "$BIN" ]]; then
    echo "error: $BIN missing — run ./scripts/flash-dp-lumen.sh (build) first" >&2
    exit 1
fi

"$SCRIPT_DIR/patch-ai-thinker-sdk.sh" "$SDK_ROOT"

cat <<'EOF'
=== Flash manual RA-08H (dp-lumen) ===

J2 (faixa vermelha = pin 1). Use a fileira de SINAIS (pins 6–10):

  Pin 6  RST          Pin 7  BOOT
  Pin 8  RX.FLASH     Pin 9  UART.TX
  Pin 10 RX.DATA (não usar p/ flash)

Ligação conversor (sem VCC):
  GND → pin 3/4/5
  TX  → pin 8
  RX  → pin 9

Entrar em boot (IO2=HIGH no reset):
  1. Jumper J2-7 (BOOT) → J2-1 (+3V3) — deixar ligado
  2. Pulse J2-6 (RST) → GND (~0,5 s)
  3. Pressione Enter IMEDIATAMENTE (script tenta SYNC por 45 s)

DTR/CTS do conversor: desligados do J2.
EOF

read -r -p "Pronto? Enter para iniciar tentativas de SYNC... " _

LOADER="$SDK_ROOT/build/scripts/tremo_loader.py"

python3 - "$LOADER" "$SERIAL_PORT" "$SERIAL_BAUDRATE" "$BIN" "$SYNC_SECONDS" <<'PY'
import importlib.util
import sys
import time
import zlib

loader_path, port, baud, bin_path, sync_seconds = sys.argv[1:6]
sync_seconds = float(sync_seconds)

spec = importlib.util.spec_from_file_location("tremo_loader", loader_path)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

tremo = mod.TremoLoader(port, 115200)
tremo.ser.timeout = 0.8

print(f"Tentando SYNC por {sync_seconds:.0f}s @ 115200...")
deadline = time.time() + sync_seconds
attempt = 0
last_err = None
while time.time() < deadline:
    attempt += 1
    try:
        tremo.drain_input(60)
        tremo.requeset(mod.TremoLoader.CMD_SYNC)
        tremo.wait_response()
        print(f"Connected on attempt {attempt}")
        break
    except mod.CmdException as exc:
        last_err = exc
        if attempt <= 5 or attempt % 5 == 0:
            print(f"  attempt {attempt}: {exc}")
        time.sleep(0.25)
else:
    tremo.ser.close()
    print(f"Connect failed after {attempt} attempts: {last_err}")
    print("Repita: BOOT em +3V3, pulse RST, Enter rápido.")
    raise SystemExit(1)

print(f"Switching to {baud} baud...")
tremo.set_baudrate(int(baud))

with open(bin_path, "rb") as f:
    data = f.read()
size = len(data)
checksum = zlib.crc32(data) & 0xFFFFFFFF
addr = 0x08000000

print(f"Erasing {size} bytes @ 0x{addr:08x}...")
tremo.erase(addr, size)
off = 0
while off < size:
    chunk = data[off : off + 512]
    tremo.flash(addr + off, chunk)
    off += len(chunk)
    if off == size or off % 4096 == 0:
        print(f"  sent {off}/{size}")
tremo.verify(addr, size, checksum)
tremo.reboot(0)
tremo.ser.close()
print("Flash OK")
PY
