#!/usr/bin/env bash
# Full flash diagnostic checklist for dp-lumen / RA-08H.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SDK_ROOT="${RA08H_SDK_PATH:-$REPO_ROOT/Ai-Thinker-LoRaWAN-Ra-08}"
BIN="$REPO_ROOT/sdk/ra08h-dp-lumen/out/ra08h-dp-lumen.bin"
PORT="${SERIAL_PORT:-/dev/ttyUSB0}"
LOADER="$SDK_ROOT/build/scripts/tremo_loader.py"

pass() { echo "  [OK]   $*"; }
fail() { echo "  [FAIL] $*"; }
warn() { echo "  [??]   $*"; }
info() { echo "  [--]   $*"; }

echo "================================================================"
echo " dp-lumen — diagnóstico completo de flash RA-08H"
echo "================================================================"
echo

echo "A) SOFTWARE / PC"
echo "----------------"

if [[ -d "$SDK_ROOT" ]]; then
    pass "SDK em $SDK_ROOT"
else
    fail "SDK ausente — clone Ai-Thinker-LoRaWAN-Ra-08"
fi

if [[ -f "$BIN" ]]; then
    pass "Firmware $(basename "$BIN") ($(wc -c <"$BIN") bytes)"
else
    fail "Binário ausente — rode ./scripts/flash-dp-lumen.sh (build)"
fi

if command -v python3 >/dev/null; then
    pass "python3 disponível"
else
    fail "python3 não encontrado"
fi

if python3 -c "import serial" 2>/dev/null; then
    pass "pyserial instalado"
else
    fail "pip install pyserial"
fi

if [[ -e "$PORT" ]]; then
    pass "Porta $PORT existe"
    udevadm info -q property -n "$PORT" 2>/dev/null | grep -E 'ID_MODEL=|ID_SERIAL=' | sed 's/^/         /' || true
else
    fail "Porta $PORT não existe — plugue o conversor USB"
fi

if [[ -e "$PORT" ]] && groups | grep -q dialout; then
    pass "Usuário no grupo dialout"
elif [[ -e "$PORT" ]]; then
    warn "Usuário pode não ter permissão serial (grupo dialout)"
fi

if [[ -e "$PORT" ]] && fuser "$PORT" >/dev/null 2>&1; then
    fail "Porta $PORT ocupada — feche monitor serial"
else
    pass "Porta $PORT livre"
fi

if [[ -f "$LOADER" ]]; then
    if grep -q "profile=None" "$LOADER"; then
        pass "tremo_loader.py patcheado (dp-lumen)"
    else
        warn "tremo_loader sem patch — rode ./scripts/patch-ai-thinker-sdk.sh"
    fi
else
    fail "tremo_loader.py não encontrado"
fi

echo
echo "B) LIGAÇÃO CONVERSOR ↔ J2 (verificar na bancada)"
echo "------------------------------------------------"
info "Conversor FT232/CH340 — NÃO ligar VCC no J2"
info "GND  conversor → J2 pin 3, 4 ou 5"
info "TX   conversor → J2 pin 8  (IO16 / UART flash)"
info "RX   conversor → J2 pin 9  (IO17 / UART TX módulo)"
info "DTR  (opcional auto) → J2 pin 7 (BOOT)"
info "CTS  (opcional auto) → J2 pin 6 (RST)"
warn "TX no pin 10 = AT only — NUNCA funciona para flash"
warn "Cabo IDC invertido 180° troca todos os pinos — confira faixa vermelha = pin 1"

echo
echo "C) ALIMENTAÇÃO DA PLACA"
echo "-----------------------"
info "RA-08H (U6) alimentado pelo +3V3 da placa (LD1117 U1)"
info "Pode usar bateria UP645 (~6V) OU rede AC — não precisa dos dois"
info "Medir multímetro: J2 pin 1 vs pin 3 → deve dar ~3,3 V"
warn "Bateria muito descarregada → 3V3 instável → flash falha"
warn "NÃO alimentar placa pelo VCC do conversor USB"

echo
echo "D) MODO BOOT (causa #1 mais provável)"
echo "-------------------------------------"
info "IO2/BOOT = HIGH no momento do reset (datasheet RA-08H)"
info "Manual: jumper J2 pin 7 → pin 1 (+3V3), manter ligado"
info "        pulse J2 pin 6 (RST) → GND ~0,5 s"
info "        rodar boot-sync-test IMEDIATAMENTE (janela ~2 s)"
info "Automático: DTR→7, CTS→6 — muitos FT232 6-pin NÃO resetam via CTS"
warn "BOOT tem pull-down 10k na placa — sem jumper 7→1 não entra em boot"

echo
echo "E) TESTE UART AO VIVO (placa ligada, sem boot)"
echo "----------------------------------------------"
if [[ -e "$PORT" ]] && ! fuser "$PORT" >/dev/null 2>&1; then
    "$SCRIPT_DIR/uart-wiring-test.py" -p "$PORT" || true
else
    warn "Pulando teste UART — porta indisponível"
fi

echo
echo "F) ÁRVORE DE DECISÃO"
echo "--------------------"
cat <<'EOF'
  uart-wiring-test silencioso (modo normal)
    → TX provavelmente no pin 8 — BOM para flash
    → Próximo: boot manual + boot-sync-test.py

  uart-wiring-test lixo repetitivo (jjjj, -----)
    → RX ativo mas TX no pin errado ou GND ruim
    → Confira TX→8, RX→9, GND, coluna correta do IDC

  uart-wiring-test AT com OK/+OK
    → TX no pin 10 (runtime) — mover TX para pin 8

  boot-sync-test SUCCESS (0xFE)
    → ./scripts/flash-dp-lumen-manual.sh

  boot-sync-test timeout
    → BOOT não entrou: jumper 7→1, pulse 6 durante o script
    → Medir 3V3 no pin 1
    → Verificar cabo IDC orientação

  flash Wrong response packet (não timeout)
    → UART OK mas fora do boot — repetir boot manual

  flash timeout zero bytes
    → UART quebrada ou TX desconectado
EOF

echo
echo "G) COMANDOS NA ORDEM"
echo "--------------------"
echo "  1. ./scripts/uart-wiring-test.py -p $PORT"
echo "  2. jumper 7→1; ./scripts/boot-sync-test.py -p $PORT -t 30  (+ pulse 6→GND)"
echo "  3. ./scripts/flash-dp-lumen-manual.sh"
echo
echo "================================================================"
