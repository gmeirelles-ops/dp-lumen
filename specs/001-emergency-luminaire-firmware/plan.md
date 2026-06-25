# Implementation Plan: Emergency Luminaire Firmware

**Branch**: `main` | **Date**: 2026-06-24 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `specs/001-emergency-luminaire-firmware/spec.md`

## Summary

Firmware for the Diponto **dp-lumen** emergency luminaire on **RA-08H** (ASR6601):
operating-mode state machine (mains / battery emergency / deep-sleep protection), ADC
sensing for mains and battery, GPIO control of lamps and indicator LEDs, button
handling with debounce, UART field diagnostics. Replaces legacy ESP-IDF scaffold.

**Implementation scope (current phase):** User Story 1 (P1) + User Story 2 (P2) +
UART diagnostics (FR-021).

**Deferred:** User Story 3 (P3) LoRa telemetry — contract sketched in
[`contracts/telemetry-lora.md`](./contracts/telemetry-lora.md); no implementation
tasks in this phase.

## Technical Context

**Language/Version**: C11 (ASR6601 SDK / GCC ARM embedded)

**Primary Dependencies**: Ai-Thinker RA-08H ASR6601 SDK (replaces ESP-IDF scaffold)

**Storage**: N/A (no NVS in this phase)

**Testing**: Pure-C unit tests for domain (`battery_band`, `button_policy`,
`luminaire_sm`) + bench validation per [quickstart.md](./quickstart.md)

**Target Platform**: RA-08H (ASR6601), hardware `hw-dp-lumen/04. Projeto` KiCad v0.0.1

**Project Type**: Embedded firmware (single MCU)

**Performance Goals**: Mains-to-battery transition ≤ 1 s (SC-002); button debounce 50 ms

**Constraints**: Fixed GPIO per constitution; analog charger not controlled by FW;
P3 LoRa excluded from build; secrets never in repo

**Scale/Scope**: Phase 1 — US1 + US2 + UART diag only

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
- [x] **Architecture**: domain separated from ASR6601 SDK / drivers
- [x] **LoRa scope**: P3 deferred — aligns with constitution MVP (ADC + switching)
- [x] **Build gate**: RA-08H / ASR6601 toolchain (documented in research.md)
- [x] **Hardware validation**: quickstart.md covers ADC, buttons, load, modes

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
│   ├── battery_band.c / battery_band.h
│   └── button_policy.c / button_policy.h
├── drivers/
│   ├── board_gpio.c / board_gpio.h
│   ├── board_adc.c / board_adc.h
│   └── debounce.c / debounce.h
├── platform/
│   └── ra08h/              # SDK init, sleep, UART, tick timer
└── diag/
    └── uart_diag.c / uart_diag.h
main/
└── main.c
tests/
└── unit/
    ├── test_battery_band.c
    ├── test_button_policy.c
    └── test_luminaire_sm.c
```

**Structure Decision**: Clean Architecture layers per constitution V; `connectivity/`
for LoRa deferred until P3. Legacy ESP-IDF `CMakeLists.txt` / `sdkconfig.defaults`
replaced during `/speckit-implement`.

## Implementation Phases

| Phase | Deliverable | Status |
|---|---|---|
| 1 — Foundation | RA-08H toolchain, GPIO/ADC drivers, UART | Planned |
| 2 — P1+P2 | State machine, buttons, LEDs, deep sleep | Planned |
| 3 — P3 LoRa | `telemetry-lora.md` sketch; `connectivity/` stub | **Deferred** |

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|---|---|---|
| Auto-on lamps in battery mode (FR-004) | Spec US1 — emergency lighting on grid loss | Manual-only toggle fails safety requirement |
| Constitution v2.0.0 implied manual battery toggle only | Spec approved after constitution | Requires constitution v2.1.0 bump in separate task |
