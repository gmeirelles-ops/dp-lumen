# Implementation Plan: Emergency Luminaire Firmware

**Branch**: `main` | **Date**: 2026-06-24 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/001-emergency-luminaire-firmware/spec.md`

## Summary

Firmware for the Diponto **dp-lumen** emergency luminaire on **RA-08H** (ASR6601):
operating-mode state machine (mains / battery emergency / deep-sleep protection), ADC
sensing for mains and battery, GPIO control of lamps and indicator LEDs, button
handling with debounce, UART field diagnostics. Replaces legacy ESP-IDF scaffold.

**Implementation scope (current phase):** User Story 1 (P1) + User Story 2 (P2) +
UART diagnostics (FR-021) **+ User Story 3 (P3) LoRa P2P telemetry** (FR-017–FR-020)
**+ flashing tooling** (devcontainer + status-bar Build/Flash/Monitor buttons).

**P3 LoRa (now active):** raw **LoRa P2P** uplink, wire-compatible with the
`ra08h-dp-comm` gateway. Payload = `product_id` (provisional `4`), AC present,
battery %, load state. Radio params and on-air framing in
[`contracts/telemetry-lora.md`](./contracts/telemetry-lora.md). No downlink (FR-020).

**Tooling:** `.devcontainer` (image `ra08h-env`, Tremo SDK at `/sdk`) and
`julynx.project-actions` status-bar buttons (Build/Clean/Flash/Monitor) driving
`make` / `make flash` in `sdk/ra08h-dp-lumen`, mirrored in `.vscode/tasks.json`.
Monitor uses 115200 baud (dp-lumen diag UART), not the reference's 9600.

## Technical Context

**Language/Version**: C11 (ASR6601 SDK / GCC ARM embedded)

**Primary Dependencies**: Tremo / ASR6601 SDK (RA-08H), built via Makefile +
`common.mk` (`TREMO_SDK_PATH`, default `/sdk` in devcontainer); LoRa stack from
the SDK (`lora/system`, `lora/system/crypto`, `lora/radio/sx126x`, `lora/driver`)
for P3 P2P uplink

**Storage**: N/A (no NVS in this phase)

**Testing**: Pure-C unit tests for domain (`battery_band`, `button_policy`,
`luminaire_sm`) and `protocol/telemetry_codec` + bench validation per
[quickstart.md](./quickstart.md); LoRa radio excluded from host tests via
`DP_LUMEN_PLATFORM_STUB`

**Target Platform**: RA-08H (ASR6601), hardware `hw-dp-lumen/04. Projeto` KiCad v0.0.1

**Project Type**: Embedded firmware (single MCU)

**Performance Goals**: Mains-to-battery transition ≤ 1 s (SC-002); button debounce
50 ms; telemetry on change ≤ 60 s (SC-005); heartbeat 15 min (FR-019)

**Constraints**: Fixed GPIO per constitution; analog charger not controlled by FW;
LoRa is uplink-only P2P (no downlink, FR-020); secrets never in repo

**Scale/Scope**: US1 + US2 + UART diag + P3 LoRa P2P uplink + flashing tooling

### ADC threshold reference (10k/1k dividers, Vref = 3.3 V, 12-bit)

Formula: `V_adc = V_in / 11`, `counts = V_adc × 4095 / 3.3`

| Signal | V_in | V_adc | counts (nom.) |
|---|---|---|---|
| VBAT charged | 6.7 V | 0.609 V | 756 |
| VBAT low | 5.8 V | 0.527 V | 654 |
| VBAT wake | 5.6 V | 0.509 V | 632 |
| VBAT critical | 5.4 V | 0.491 V | 609 |
| +12V mains | 12.0 V | 1.091 V | 1353 |
| Mains enter (+11V) | 11.0 V | 1.000 V | 1241 |
| Mains exit (+9.9V) | 9.9 V | 0.900 V | 1117 |

Calibrate on hardware during `/speckit-implement`; see [research.md](./research.md).

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify compliance with `.specify/memory/constitution.md` (dp-lumen v2.0.0):

- [x] **Hardware contracts**: GPIO mapping RA-08H U6 unchanged; ADC dividers per schematic
- [x] **Mode separation**: `btn.test` / `btn.power` mutual exclusion; `led.status` rules
- [x] **Load switching**: LED1/LED2 via `SPOTLIGHT.EN` (IO14) only
- [x] **Charger**: firmware does not control analog charger
- [x] **Architecture**: domain/protocol pure; radio isolated in `connectivity`
- [x] **LoRa scope**: P3 now active as **uplink-only LoRa P2P** (no downlink, FR-020);
      RF params + framing are a documented contract ([telemetry-lora.md](./contracts/telemetry-lora.md))
- [x] **RF switch pins (new)**: `GPIOD_11` / `GPIOA_10` are RA-08H module-internal
      antenna-switch controls inherited from the `ra08h-dp-comm` reference; no conflict
      with constitution app GPIO; confirm vs schematic on hardware (justified in research.md)
- [x] **Build gate**: RA-08H / ASR6601 Tremo SDK (`make`); host stub + unit tests
- [x] **Hardware validation**: quickstart.md covers ADC, buttons, load, modes, and
      LoRa uplink reception on the `ra08h-dp-comm` gateway

## Project Structure

### Documentation (this feature)

```text
specs/001-emergency-luminaire-firmware/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── uart-diagnostics.md
│   └── telemetry-lora.md
└── tasks.md              # Phase 2 — /speckit-tasks (not created here)
```

### Source Code (repository root)

```text
components/
├── domain/
│   ├── luminaire_sm.c / luminaire_sm.h
│   ├── battery_band.c / battery_band.h     # + battery_pct_from_mv()
│   ├── button_policy.c / button_policy.h
│   └── telemetry.c / telemetry.h           # NEW: snapshot + change detection (pure)
├── protocol/
│   └── telemetry_codec.c / telemetry_codec.h   # NEW: encode + checksum + crypt (pure)
├── connectivity/
│   └── lora_radio.c / lora_radio.h         # NEW: SX126x P2P TX (device only)
├── drivers/
│   ├── board_gpio.c / board_gpio.h
│   ├── board_adc.c / board_adc.h
│   └── debounce.c / debounce.h
├── platform/
│   └── ra08h/              # SDK init (+LoRa clocks), sleep, UART, tick timer
└── diag/
    └── uart_diag.c / uart_diag.h
main/
└── main.c
tests/
└── unit/
    ├── test_battery_band.c     # + battery_pct cases
    ├── test_button_policy.c
    ├── test_luminaire_sm.c
    └── test_telemetry_codec.c  # NEW: frame/checksum/crypt vs gateway

# Tooling / build
.devcontainer/devcontainer.json            # NEW: image ra08h-env, SDK at /sdk
.vscode/.project-actions.json              # NEW: Build/Clean/Flash/Monitor buttons
.vscode/tasks.json                         # mirrored tasks + host stub tasks
sdk/ra08h-dp-lumen/Makefile                # extend: connectivity/protocol + LoRa SDK
```

**Structure Decision**: Clean Architecture layers per constitution V. `connectivity/`
(radio) and `protocol/` (pure codec) added for P3. Domain stays radio-free; codec is
host-tested. Device firmware builds with the Tremo SDK Makefile at
`sdk/ra08h-dp-lumen/`; host `CMakeLists.txt` builds domain + protocol + unit tests
only (radio excluded via `DP_LUMEN_PLATFORM_STUB`).

## Implementation Phases

| Phase | Deliverable | Status |
|---|---|---|
| 1 — Foundation | RA-08H toolchain, GPIO/ADC drivers, UART | Done |
| 2 — P1+P2 | State machine, buttons, LEDs, deep sleep | Done |
| 3 — Tooling | `.devcontainer`, Project Actions buttons, tasks.json | Planned |
| 4 — P3 LoRa | `telemetry` snapshot, `telemetry_codec`, `lora_radio` P2P TX, Makefile/LoRa-SDK wiring, main-loop integration | Planned |

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|---|---|---|
| Auto-on lamps in battery mode (FR-004) | Spec US1 — emergency lighting on grid loss | Manual-only toggle fails safety requirement |
| Constitution v2.0.0 implied manual battery toggle only | Spec approved after constitution | Requires constitution v2.1.0 bump in separate task |
| RF antenna-switch GPIO (`GPIOD_11`, `GPIOA_10`) not in constitution GPIO table | Required to drive the RA-08H module antenna switch for any LoRa TX | Pins are module-internal (not app I/O) and proven on the same module by `ra08h-dp-comm`; documented in research.md; confirm vs schematic on hardware |
| `product_id = 4` hard-coded, no per-device id | Spec FR-017 — provisional product id pending definition | A full provisioning scheme is out of scope this version; tracked as a risk |
| `crypt` (bitwise NOT) is obfuscation, not encryption | Required for byte-exact compatibility with the `ra08h-dp-comm` gateway | Real encryption would require a coordinated gateway change; out of scope |
