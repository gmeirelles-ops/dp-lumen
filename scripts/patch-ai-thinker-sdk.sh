#!/usr/bin/env bash
# Patch Ai-Thinker ASR6601 SDK for gcc-arm-none-eabi 10+ on Linux.
# GCC 14 no longer exposes <stdint-gcc.h> as a standalone include.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="${1:-}"

if [[ -z "$SDK_ROOT" ]]; then
    SDK_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)/Ai-Thinker-LoRaWAN-Ra-08"
fi

TARGET="$SDK_ROOT/platform/system/printf-stdarg.c"

if [[ ! -f "$TARGET" ]]; then
    echo "error: SDK not found at $SDK_ROOT" >&2
    echo "usage: $0 [/path/to/Ai-Thinker-LoRaWAN-Ra-08]" >&2
    exit 1
fi

if grep -q '#include <stdint-gcc.h>' "$TARGET"; then
    sed -i 's/#include <stdint-gcc.h>/#include <stdint.h>/' "$TARGET"
    echo "patched: $TARGET (stdint-gcc.h -> stdint.h)"
else
    echo "already patched or unchanged: $TARGET"
fi

LOADER="$SDK_ROOT/build/scripts/tremo_loader.py"
if [[ -f "$LOADER" ]]; then
    python3 "$SCRIPT_DIR/patch-tremo-loader.py" "$LOADER"
fi
