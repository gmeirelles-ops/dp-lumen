#!/usr/bin/env bash
# Flash automático RA-08H — DTR/RTS ou cap DTR->RST via tremo_loader.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cat <<'EOF'
=== Flash automático dp-lumen (RA-08H) ===

Seu conversor é FT232R (6 pinos). Escolha UMA das ligações:

Opção A — DTR + RTS (se tiver fio RTS no módulo):
  GND → J2 3/4/5
  TX  → J2 8
  RX  → J2 9
  DTR → J2 7 (BOOT)
  RTS → J2 6 (RST)    (no cabo pode estar como CTS)

Opção B — só DTR + capacitor (recomendado FT232 6-pin):
  GND → J2 3/4/5
  TX  → J2 8
  RX  → J2 9
  DTR → J2 7 (BOOT)
  DTR ──[100nF]── J2 6 (RST)
  J2 6 ──[10k]── J2 1 (+3V3)

  export TREMO_RESET_MODE=dtr_cap

VCC do conversor: NÃO ligar. Lógica 3V3.
Placa ligada (rede ou bateria).

O script alterna dtr_rts / dtr_cap quando TREMO_RESET_MODE=auto (padrão).
EOF

export FLASH_ONLY="${FLASH_ONLY:-1}"
export TREMO_RESET_MODE="${TREMO_RESET_MODE:-auto}"
exec "$SCRIPT_DIR/flash-dp-lumen.sh"
