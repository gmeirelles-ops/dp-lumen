#!/usr/bin/env bash
# Build and flash dp-lumen firmware on RA-08H.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SDK_ROOT="${RA08H_SDK_PATH:-$REPO_ROOT/Ai-Thinker-LoRaWAN-Ra-08}"
PROJECT_DIR="${RA08H_PROJECT_DIR:-$REPO_ROOT/sdk/ra08h-dp-lumen}"
SERIAL_PORT="${SERIAL_PORT:-}"

if [[ -z "$SERIAL_PORT" ]]; then
    if [[ -e /dev/ttyUSB0 ]]; then
        SERIAL_PORT=/dev/ttyUSB0
    elif [[ -e /dev/ttyACM0 ]]; then
        SERIAL_PORT=/dev/ttyACM0
    else
        SERIAL_PORT=/dev/ttyUSB0
    fi
fi

echo "Using serial port: $SERIAL_PORT"

if [[ ! -e "$SERIAL_PORT" ]]; then
    echo "error: port $SERIAL_PORT not found. Plug FTDI and run: ls /dev/ttyUSB*" >&2
    exit 1
fi

if fuser "$SERIAL_PORT" >/dev/null 2>&1; then
    echo "error: $SERIAL_PORT busy — close serial monitor before flashing." >&2
    fuser -v "$SERIAL_PORT" 2>&1 || true
    exit 1
fi

"$SCRIPT_DIR/patch-ai-thinker-sdk.sh" "$SDK_ROOT"

cd "$SDK_ROOT"
# shellcheck disable=SC1091
source build/envsetup.sh
cd "$PROJECT_DIR"

if [[ "${FLASH_ONLY:-}" != "1" ]]; then
    make clean 2>/dev/null || true
    make -j"$(nproc)"
fi
make SERIAL_PORT="$SERIAL_PORT" flash
