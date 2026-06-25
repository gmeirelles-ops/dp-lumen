# Implementation Notes: 001-emergency-luminaire-firmware

**Date**: 2026-06-24  
**Scope**: P1 + P2 + UART diagnostics (P3 LoRa deferred)

## Build results

| Target | Command | Result |
|---|---|---|
| Stub (host gcc) | `cmake -B build -S . && cmake --build build` | **PASS** |
| Unit tests | `ctest --test-dir build` | **PASS** (3/3) |
| RA-08H SDK | `RA08H_SDK_PATH` unset on build host | **SKIPPED** — platform HAL stubs only |

## Hardware validation pending (QS-03–QS-07)

Requires bench with dp-lumen board, adjustable VBAT, mains simulation, and authorized flash:

- **QS-03**: Grid loss → lamps auto-on within 1 s (ADC mains hysteresis 200 ms + tick 50 ms)
- **QS-04**: `btn.power` toggle in battery mode
- **QS-05**: `led.battery` at VBAT ≤ 5.8 V
- **QS-07**: Deep sleep at ≤ 5.4 V; wake on mains or VBAT > 5.6 V

Domain logic covered by `tests/unit/test_luminaire_sm.c`; ADC scaling and GPIO polarity need bench confirmation.

## Quickstart cross-check (T036)

| Scenario | Domain / stub | Hardware |
|---|---|---|
| QS-01 Mains idle | `test_qs01_mains_idle` | Pending |
| QS-02 Test toggle | `test_qs02_test_toggle_mains` | Pending |
| QS-03 Grid loss | `test_qs03_grid_loss_auto_on` | Pending |
| QS-04 Power battery | `test_qs04_power_toggle_battery` | Pending |
| QS-05 Low batt LED | `test_qs05_low_battery_led` | Pending |
| QS-06 Charged LED mains | Covered by QS-01 (`led.load` when charged) | Pending |
| QS-07 Deep sleep | `test_qs07_deep_sleep_wake` | Pending wake current |
| QS-08 UART | `uart_diag.c` format per contract | Pending serial capture |

## Constitution compliance (T037)

- GPIO map unchanged (IO4/5/7/8/9/11/14/15 per constitution v2.0.0)
- No LoRa keys, DevEUI, or RF secrets in repo
- No `components/connectivity/` (P3 deferred)
- Divergence documented: spec auto-on lamps in battery mode vs constitution v2.0.0 manual-only implication — suggest constitution v2.1.0

## Remaining risks (T038)

1. **ADC calibration**: divider 10k/1k assumed; bench `adc_offset`/scale may be needed in `platform_adc_read()`.
2. **Constitution v2.1.0**: align auto-on battery behavior and deferred LoRa telemetry.
3. **RA-08H HAL**: `platform_init.c` / `platform_sleep.c` are stubs until SDK linked via `RA08H_SDK_PATH`.
4. **Hardware flash**: requires explicit user authorization; not performed in this phase.
