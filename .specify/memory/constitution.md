<!--
Sync Impact Report
- Version change: 1.0.0 → 2.0.0
- Modified principles:
  - I. Hardware Contracts Are Stable (expanded with dp-lumen GPIO table)
  - II. Spec First, Code Second (retained)
  - III. Field Diagnosability Is Required (adapted for UART/RA-08H)
  - IV. Embedded Clean Architecture → V. Embedded Clean Architecture (RA-08H)
  - V. Validation Before Done (RA-08H toolchain gate)
- Added principles:
  - II. Emergency Luminaire Safety & Mode Separation (new)
- Removed sections:
  - Template Constraints (ESP32-family generic)
- Added sections:
  - Product & Hardware Constraints
- Templates:
  - .specify/templates/plan-template.md ✅ updated
  - .specify/templates/spec-template.md ✅ updated
  - .specify/templates/tasks-template.md ✅ updated
  - README.md ✅ updated
  - AGENTS.md ✅ updated
- Deferred TODOs:
  - TODO(ADC_THRESHOLDS): Calibrate exact VBAT voltages for 10% low-battery
    and "charged" states in /speckit-specify using UP645 datasheet and
    schematic divider values (provisional: low ≤5.9V, charged ≥6.4V).
-->

# dp-lumen Constitution

Firmware constitution for the Diponto **dp-lumen** emergency luminaire: mains-powered
unit with rechargeable **UP645** 6V lead-acid battery, dual 6V LED loads, and
**RA-08H** (ASR6601) LoRa MCU. Hardware reference: `hw-dp-lumen/04. Projeto`
KiCad rev **v0.0.1**.

## Core Principles

### I. Hardware Contracts Are Stable

GPIO pinout, electrical function, ADC divider topology, connector pinout,
load-switch topology and RA-08H bootstrapping constraints are compatibility
contracts. They MUST NOT change without explicit requirement, documented
rationale, migration impact and validation plan.

Authoritative GPIO mapping (RA-08H U6, netlist `production/netlist.ipc`):

| KiCad signal | RA-08H pin | MCU IO | Electrical function |
|---|---|---|---|
| `ADC_IN1` | 2 | IO8 | Battery voltage sense (+VBAT, R12/R14 divider) |
| `ADC_IN0` | 3 | IO11 | Mains presence via +12V rail (R13/R15 divider) |
| `BTN.TEST` | 4 | IO9 | Test button S2 |
| `LED.STATUS` | 5 | IO4 | Mains status LED D9 (blue) |
| `LED.BATTERY` | 6 | IO5 | Low-battery LED D10 (red) |
| `LED.LOAD` | 7 | IO7 | Charged-battery LED D11 (green) |
| `SPOTLIGHT.EN` | 14 | IO14 | LED1/LED2 load enable via Q5 MOSFET |
| `BTN.POWER` | 13 | IO15 | Power button S1 |
| `LORA.BOOT` | 15 | IO2 | Boot / programming |
| `LORA.~{RST}` | 16 | RSTN | Module reset |

Connectors J1 (AC in), J3 (battery), J4/J5 (LED1/LED2), J6 (antenna) and
programming header J2 (UART, BOOT, RST) are part of the same contract.

### II. Emergency Luminaire Safety & Mode Separation

The firmware MUST distinguish **mains-present** and **battery-only** operating
modes using ADC-based mains detection (`ADC_IN0` / +12V rail).

**Mains-present mode** (grid power available):

- `led.status` MUST be ON.
- `btn.test` MUST toggle LED1/LED2 load ON/OFF (test function).
- `btn.power` MUST be ignored.
- Battery charging is handled by analog hardware (LM317 + VIPer22); firmware
  MUST NOT attempt to control the charger.

**Battery-only mode** (no grid power):

- `led.status` MUST be OFF.
- `btn.power` MUST toggle LED1/LED2 load ON/OFF.
- `btn.test` MUST be ignored.

**Indicator LEDs** (evaluated continuously from `ADC_IN1` / VBAT):

- `led.battery` MUST be ON when battery state-of-charge is below **10%**
  (derived from VBAT; exact voltage thresholds calibrated in spec).
- `led.load` MUST be ON when battery is considered **charged** (threshold
  calibrated in spec).

Load switching MUST go through `SPOTLIGHT.EN` only. Mode transitions MUST
leave load and indicators in a safe, deterministic state.

### III. Spec First, Code Second

Feature work follows the Spec Kit flow: constitution, specify, clarify when
useful, checklist, plan, tasks and implement. Specification, planning and
task-generation phases MUST NOT implement code. Implementation MUST follow
approved `spec.md`, `plan.md` and `tasks.md`. If implementation reveals a
requirement, architecture or hardware-contract change, update the Spec Kit
artifacts before continuing.

### IV. Field Diagnosability Is Required

Firmware MUST produce actionable diagnostics for real hardware and field
support via UART (J2). Logs MUST expose explicit states: mains present/absent,
battery SOC band (low / normal / charged), load ON/OFF, active button mode and
ignored-button events. LoRa RF parameters and keys MUST NOT appear in logs.
Unknown errors are acceptable only after available subsystem-specific causes
have been checked.

### V. Embedded Clean Architecture, Pragmatically Applied (RA-08H)

Application/domain logic MUST remain separated from RA-08H / ASR6601 SDK
infrastructure where practical:

- `domain` — operating modes, button policy, SOC rules, state machine.
- `drivers` — GPIO, ADC, debounce, `SPOTLIGHT.EN` control.
- `platform` — ASR6601 SDK, clock, UART, timers.
- `connectivity` — LoRa telemetry (future scope; not required for MVP).

Domain MUST NOT include SDK headers for GPIO/ADC directly when a driver
boundary is practical. Abstractions MUST reduce real coupling, improve tests or
clarify lifecycle.

### VI. Validation Before Done

Every code change MUST be validated with the strongest practical check
available. The default firmware build gate is the **RA-08H / ASR6601**
toolchain build (Ai-Thinker SDK or project-equivalent). When firmware size,
memory or LoRa behavior may be affected, run size/memory analysis with the
same toolchain.

Hardware validation is REQUIRED when behavior touches ADC thresholds, button
handling, load switching or mode transitions. Report what still requires
real-device testing on dp-lumen hardware.

## Product & Hardware Constraints

Project-specific contracts for dp-lumen:

- **MCU**: RA-08H (Ai-Thinker), ASR6601 — NOT ESP32.
- **SDK**: ASR6601 / Ai-Thinker RA-08H toolchain (ESP-IDF scaffold in repo is
  legacy and MUST be migrated before feature implementation).
- **Battery**: UP645, 6V lead-acid; charger 6V / 450mA (hardware).
- **Loads**: LED1 and LED2, 6V each, switched via Q5 / `SPOTLIGHT.EN`.
- **Mains detection**: `ADC_IN0` sensing +12V rail — not direct AC GPIO.
- **LoRa**: integrated in RA-08H; RF parameters (frequency, SF, TX power,
  payload schema) are future contracts — MVP requires ADC + load switching only.
- **Out of scope for this product**: Wi-Fi, BLE, MQTT, TLS, NVS provisioning,
  ESP-ADF audio, ESP32 OTA.

Secrets, credentials, LoRa keys and production tokens MUST NEVER be committed
to the repository or feature specs.

## Cross-Platform Development

Spec Kit scripts support Windows, Linux and macOS via
`.specify/scripts/bash/` and `.specify/scripts/powershell/`.

- Use Bash scripts on Linux, macOS, WSL or Git Bash.
- Use PowerShell scripts on native Windows.
- RA-08H firmware builds typically run on Linux; keep generated specs and
  tasks portable — avoid OS-specific paths unless the feature explicitly
  targets that OS.

## Governance

This constitution supersedes informal project habits for dp-lumen. Amendments
require a documented reason, affected compatibility contracts, migration impact
and validation plan. `AGENTS.md` may add operational guidance, but it MUST NOT
weaken these principles.

Versioning policy:

- **MAJOR**: backward-incompatible governance, principle removal or MCU/platform
  redefinition.
- **MINOR**: new principle or materially expanded product guidance.
- **PATCH**: clarifications, wording, non-semantic refinements.

**Version**: 2.0.0 | **Ratified**: 2026-06-24 | **Last Amended**: 2026-06-24
