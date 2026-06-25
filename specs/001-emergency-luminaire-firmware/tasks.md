# Tasks: Emergency Luminaire Firmware

**Input**: Design documents from `specs/001-emergency-luminaire-firmware/`

**Prerequisites**: [plan.md](./plan.md), [spec.md](./spec.md), [research.md](./research.md), [data-model.md](./data-model.md), [contracts/](./contracts/), [quickstart.md](./quickstart.md)

**Implementation scope**: User Story 1 (P1) + User Story 2 (P2) + UART diagnostics. User Story 3 (P3) LoRa is **deferred** — see Phase Future below.

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: US1 or US2 — user story phase tasks only
- Every implementation task includes an exact file path

## Path Conventions

```text
components/domain/          # luminaire_sm, battery_band, button_policy
components/drivers/           # board_gpio, board_adc, debounce
components/platform/ra08h/  # SDK init, timer, sleep, UART
components/diag/            # uart_diag
main/main.c
tests/unit/
```

## Dependencies

```text
Phase 1 (Setup)
    └── Phase 2 (Foundational) — blocks all user stories
            └── Phase 3 (US1 — MVP)
                    └── Phase 4 (US2)
                            └── Phase 5 (Polish)
Phase Future (P3 LoRa) — no tasks; not on critical path
```

US2 extends `luminaire_sm.c` built in US1; US1 is independently testable after Foundational (QS-03–QS-07).

## Parallel Execution Examples

```bash
# After T003 — foundational drivers/domain headers in parallel:
T006 board_gpio.h  ||  T008 board_adc.h  ||  T010 debounce.c  ||  T011 battery_band.c

# After T019 — US1 tests while wiring GPIO outputs:
T026 test_luminaire_sm.c  ||  T024 GPIO apply in luminaire_sm integration

# US2 — extend tests after state machine rules land:
T033 test_luminaire_sm.c (mains cases)  ||  T035 uart_diag.c enhancements
```

## Implementation Strategy

1. Complete **Phase 1–2** before any user story work (RA-08H toolchain + drivers + pure domain).
2. **MVP = Phase 3 (US1)** — emergency battery operation delivers core safety value.
3. Add **Phase 4 (US2)** for mains mode, test button, and charge indicators.
4. **Phase 5** polish and hardware validation notes.
5. **Do not** implement P3 LoRa in this task list.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project structure and RA-08H build scaffold (replace ESP-IDF legacy).

- [x] T001 Create directory tree `components/domain/`, `components/drivers/`, `components/platform/ra08h/`, `components/diag/`, and `tests/unit/` per plan.md
- [x] T002 Replace root `CMakeLists.txt` for RA-08H ASR6601 SDK build (remove `IDF_PATH` / ESP-IDF `project.cmake`)
- [x] T003 Add `components/CMakeLists.txt` registering domain, drivers, platform, and diag subdirectories
- [x] T004 [P] Update `main/CMakeLists.txt` and replace `main/main.c` with RA-08H application skeleton (init + tick loop placeholder)
- [x] T005 [P] Document `RA08H_SDK_PATH` build prerequisite in `README.md` (no local secrets or absolute paths committed)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: GPIO, ADC, debounce, pure domain helpers, platform tick, UART diag shell, and host unit-test harness. **No user story work until this phase is complete.**

- [x] T006 [P] Define RA-08H pin map and GPIO enums in `components/drivers/board_gpio.h` (constitution GPIO table)
- [x] T007 [P] Implement `components/drivers/board_gpio.c` — init, read `btn.power`/`btn.test`, write `led.status`/`led.battery`/`led.load`, `SPOTLIGHT.EN`
- [x] T008 [P] Define ADC thresholds and mV conversion constants in `components/drivers/board_adc.h` per research.md
- [x] T009 Implement `components/drivers/board_adc.c` — 8-sample average, `board_adc_read_vbat_mv()`, `board_adc_mains_present()` with 200 ms hysteresis
- [x] T010 [P] Implement `components/drivers/debounce.c` and `components/drivers/debounce.h` — 50 ms debounce, active-low buttons
- [x] T011 [P] Implement `components/domain/battery_band.c` and `components/domain/battery_band.h` — pure `battery_band_from_mv()` with 5.9 V leave-low hysteresis
- [x] T012 [P] Implement `components/domain/button_policy.c` and `components/domain/button_policy.h` — FR-008 through FR-011 ignore/accept rules
- [x] T013 Implement `components/platform/ra08h/platform_init.c`, `platform_tick.c`, and headers — 50 ms periodic tick, UART 115200 8N1 on J2
- [x] T014 [P] Implement `components/diag/uart_diag.c` and `components/diag/uart_diag.h` per contracts/uart-diagnostics.md log format
- [x] T015 [P] Add host unit-test runner under `tests/unit/CMakeLists.txt` (or `tests/unit/Makefile`) linking domain sources without hardware
- [x] T016 [P] Add `tests/unit/test_battery_band.c` covering Charged/Normal/Low/Critical thresholds
- [x] T017 [P] Add `tests/unit/test_button_policy.c` covering mains vs battery button ignore rules
- [x] T018 Run RA-08H SDK build from repo root and confirm compile succeeds with foundational stubs

**Checkpoint**: Foundation ready — US1 implementation can begin.

---

## Phase 3: User Story 1 — Operação de emergência na bateria (Priority: P1)

**Goal**: On grid loss, lamps auto-on; `btn.power` toggles load; low-battery and deep-sleep protection; wake on mains or VBAT > 5.6 V.

**Independent Test**: quickstart.md QS-03, QS-04, QS-05, QS-07 (bench); domain tests QS-03–05, QS-07 scenarios.

### Implementation

- [x] T019 [US1] Define `luminaire_state_t`, `luminaire_inputs_t`, and API in `components/domain/luminaire_sm.h` per data-model.md
- [x] T020 [US1] Implement `MainsPresent` → `BatteryEmergency` transition with auto LAMP ON in `components/domain/luminaire_sm.c` (FR-004)
- [x] T021 [US1] Implement `btn.power` load toggle in battery mode in `components/domain/luminaire_sm.c` using button_policy
- [x] T022 [US1] Implement `led.battery` and battery band indicators for Low/Critical in `components/domain/luminaire_sm.c`
- [x] T023 [US1] Implement `DeepSleepProtection` at VBAT ≤ 5.4 V and wake on mains or VBAT > 5.6 V in `components/domain/luminaire_sm.c` and `components/platform/ra08h/platform_sleep.c`
- [x] T024 [US1] Apply lamp and indicator GPIO outputs from `luminaire_state_t` in `components/domain/luminaire_sm.c` via board_gpio
- [x] T025 [US1] Wire `main/main.c` 50 ms loop: ADC → debounce → `luminaire_sm_tick()` → GPIO + uart_diag
- [x] T026 [P] [US1] Add `tests/unit/test_luminaire_sm.c` for QS-03, QS-04, QS-05, QS-07 domain scenarios
- [x] T027 [US1] Run RA-08H build after US1 and record pass/fail in implementation notes
- [x] T028 [US1] Document pending hardware validation for QS-03 through QS-07 in feature implementation summary

---

## Phase 4: User Story 2 — Operação com rede presente (Priority: P2)

**Goal**: Mains idle (lamps off, `led.status` on), `btn.test` lamp toggle, `led.load` when charged, ignore `btn.power`, TestSession on mains return.

**Independent Test**: quickstart.md QS-01, QS-02, QS-06.

### Implementation

- [x] T029 [US2] Implement `MainsPresent` rules — lamps default OFF, `led.status` ON in `components/domain/luminaire_sm.c` (FR-003, FR-005)
- [x] T030 [US2] Implement `btn.test` toggle and `TestSession` state in `components/domain/luminaire_sm.c` (US2 scenario 6)
- [x] T031 [US2] Implement `led.load` only when mains present and VBAT ≥ 6.7 V in `components/domain/luminaire_sm.c` (FR-007)
- [x] T032 [US2] Implement battery→mains transition — lamps OFF unless TestSession active in `components/domain/luminaire_sm.c`
- [x] T033 [P] [US2] Extend `tests/unit/test_luminaire_sm.c` with QS-01, QS-02, QS-06 scenarios
- [x] T034 [US2] Run RA-08H build after US2 and confirm compile/link success
- [x] T035 [US2] Emit UART diag lines on mode change and ignored-button events in `components/diag/uart_diag.c` (FR-021)

---

## Phase 5: Polish and Cross-Cutting Concerns

**Purpose**: Documentation alignment, review, and field-validation handoff.

- [x] T036 Cross-check `specs/001-emergency-luminaire-firmware/quickstart.md` scenarios QS-01–QS-08 against implemented firmware behavior
- [x] T037 Review `git diff` for constitution compliance — GPIO map unchanged, no LoRa secrets in repo
- [x] T038 Report remaining risks: ADC bench calibration, constitution v2.1.0 alignment, hardware flash validation (requires explicit user authorization)

---

## Phase Future: P3 LoRa Telemetry (Deferred)

**Not in current implementation.** No tasks below — do not create `components/connectivity/` in this phase.

| Item | Status |
|---|---|
| US3 Telemetria LoRa (spec.md) | Deferred |
| FR-017, FR-018, FR-019, SC-005 | Deferred |
| Target contract | [contracts/telemetry-lora.md](./contracts/telemetry-lora.md) |
| Domain hook | `TelemetrySnapshot` in data-model.md — populate only; no RF transmit |
| Remote commands | Out of scope per spec |

Implement in a future feature after P1+P2 are validated on hardware.

---

## Task Summary

| Phase | Task IDs | Count |
|---|---|---|
| Setup | T001–T005 | 5 |
| Foundational | T006–T018 | 13 |
| US1 (P1) | T019–T028 | 10 |
| US2 (P2) | T029–T035 | 7 |
| Polish | T036–T038 | 3 |
| **Total** | T001–T038 | **38** |
| P3 Future | — | 0 implementation tasks |

**MVP scope**: Complete through Phase 3 (T028) for emergency battery operation.
