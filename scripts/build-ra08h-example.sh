#!/usr/bin/env bash
# Build an Ai-Thinker SDK example (default: uart_printf).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SDK_ROOT="${RA08H_SDK_PATH:-$REPO_ROOT/Ai-Thinker-LoRaWAN-Ra-08}"
PROJECT_DIR="${RA08H_PROJECT_DIR:-$REPO_ROOT/sdk/ra08h-dp-lumen}"

if [[ $# -gt 0 && "$1" != clean && "$1" != flash ]]; then
    PROJECT_DIR="$1"
    shift
fi

if [[ ! -f "$SDK_ROOT/build/envsetup.sh" ]]; then
    echo "error: SDK not found. Clone first:" >&2
    echo "  git clone --recursive https://github.com/Ai-Thinker-Open/Ai-Thinker-LoRaWAN-Ra-08.git" >&2
    echo "  export RA08H_SDK_PATH=\$PWD/Ai-Thinker-LoRaWAN-Ra-08" >&2
    exit 1
fi

"$SCRIPT_DIR/patch-ai-thinker-sdk.sh" "$SDK_ROOT"

cd "$SDK_ROOT"
# shellcheck disable=SC1091
source build/envsetup.sh
cd "$PROJECT_DIR"
make "$@"
