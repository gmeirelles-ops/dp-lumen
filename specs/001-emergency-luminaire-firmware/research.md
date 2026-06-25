# Research: Emergency Luminaire Firmware

**Feature**: `001-emergency-luminaire-firmware`  
**Date**: 2026-06-24  
**Phase**: 0 — resolves Technical Context unknowns for P1+P2

## 1. SDK / Toolchain (RA-08H / ASR6601)

**Decision**: Use the **Ai-Thinker ASR6601 SDK** (RA-08H official toolchain), not a
generic STM32CubeWL port, as the primary build and HAL layer.

**Rationale**: RA-08H integrates ASR6601 with LoRa radio firmware; the Ai-Thinker SDK
provides board support, LoRa stack hooks (for future P3), and programming flow aligned
with J2 (UART/BOOT/RST). Constitution mandates RA-08H as platform.

**Alternatives considered**:
- *STM32CubeWL*: closer to STM32WL5x but lacks Ai-Thinker LoRa integration and
  RA-08H module pin mapping out of the box; higher bring-up cost.
- *Keep ESP-IDF scaffold*: incompatible with on-board MCU; rejected.

**Action for implement**: Install SDK per Ai-Thinker documentation; replace root
`CMakeLists.txt` with SDK project layout; verify `idf.py`-free build gate.

---

## 2. ADC (ASR6601 — IO8 / IO11)

**Decision**: 12-bit ADC, **Vref = 3.3 V** (module `VCC`), channels mapped per
constitution: `IO11` (`ADC_IN0`) for +12V mains sense, `IO8` (`ADC_IN1`) for +VBAT.

**Rationale**: Schematic uses identical dividers R_upper=10k, R_lower=1k (R12/R14
for VBAT, R13/R15 for +12V). `V_adc = V_in × R_lower / (R_upper + R_lower) = V_in / 11`.

**Nominal count thresholds** (`counts = V_adc × 4095 / 3.3`):

| Quantity | V_in | counts |
|---|---|---|
| VBAT charged (≥ 6.7 V) | 0.609 V | ≥ 756 |
| VBAT low (≤ 5.8 V) | 0.527 V | ≤ 654 |
| VBAT wake (> 5.6 V) | 0.509 V | > 632 |
| VBAT critical (≤ 5.4 V) | 0.491 V | ≤ 609 |
| Mains present (+12 V nom.) | 1.091 V | ≥ 1353 |
| Mains enter hysteresis (+11 V) | 1.000 V | ≥ 1241 |
| Mains exit hysteresis (+9.9 V) | 0.900 V | ≤ 1117 |

**Sampling**: Average **8 consecutive samples** every **50 ms** tick; band and mains
detection use averaged value to reduce noise.

**Alternatives considered**:
- *External comparator for AC_DETECT*: not on schematic; rejected.
- *Single threshold without averaging*: rejected — noisy near band edges.

**Calibration**: Store `adc_offset` / scale in platform config after first bench
measurement; domain uses millivolts via `board_adc_read_mv()`.

---

## 3. AC_DETECT Hysteresis (FR-002)

**Decision**: Mains **present** when +12V ADC ≥ 1241 counts (~11 V); mains **absent**
when ≤ 1117 counts (~9.9 V). Require **stable reading for 200 ms** before mode change.

**Rationale**: Prevents lamp/indicator flapping when +12V rail droops at threshold.
200 ms × 50 ms tick = 4 consistent samples in band.

**Alternatives considered**:
- *No time filter*: fails edge-case flapping in spec.
- *1 s debounce*: too slow for SC-002 (≤ 1 s lamp on after grid loss).

---

## 4. Deep Sleep (FR-014 / FR-015)

**Decision**: On VBAT ≤ 5.4 V (`Critical`), force `LAMP_CTRL` OFF, turn off non-essential
outputs, enter **Stop / low-power mode** via ASR6601 SDK. Wake on:
1. Mains restored (`AC_DETECT` present after hysteresis), or
2. VBAT > 5.6 V (counts > 632) measured on **periodic wake** every **2 s**.

**Rationale**: Spec requires deep sleep below 5.4 V and dual wake condition. Periodic
2 s wake allows battery recovery detection without mains; mains wake can use EXTI or
ADC poll after wake.

**Alternatives considered**:
- *Shutdown with no VBAT wake*: cannot recover from partial charge without mains.
- *btn.power wake*: not in user decision; deferred.

**Implement note**: Confirm lowest-power Stop mode that preserves RAM for state in
SDK docs; UART disabled in sleep, re-init on wake.

---

## 5. GPIO Polarity

**Decision** (from schematic topology — confirm on bench):

| Signal | IO | Active level | Notes |
|---|---|---|---|
| `SPOTLIGHT.EN` | IO14 | **HIGH** = lamps ON | N-ch MOSFET Q5 (JSM80N03D) gate drive via R24 |
| `LED.STATUS/BATTERY/LOAD` | IO4/5/7 | **HIGH** = LED ON | BC817 NPN low-side switch |
| `BTN.POWER` | IO15 | **LOW** = pressed | Pull-up R22 to +3V3 |
| `BTN.TEST` | IO9 | **LOW** = pressed | Pull-up R23 to +3V3 |

**Rationale**: Tactile switches tie to GND; pull-ups on R22/R23. Indicator transistors
are low-side switches — GPIO high turns LED on.

**Alternatives considered**: Inverted logic in driver layer only if bench proves
otherwise; `board_gpio` centralizes polarity.

---

## 6. LoRa Telemetry (P3 — active, LoRa P2P)

**Decision**: Implement P3 as **raw LoRa P2P** uplink, byte-compatible with the
`ra08h-dp-comm` gateway. Radio params: 915 MHz, BW 500 kHz, SF7, CR 4/5, 22 dBm,
preamble 8, CRC on, IQ off, TX timeout 3000 ms. On-air frame =
`crypt([checksum, payload])` with `checksum_calc` (two's complement) and `crypt`
(bitwise NOT), copied verbatim from the gateway. Payload = 3 bytes
`[product_id, status_flags, battery_pct]`. Full contract in
[`contracts/telemetry-lora.md`](./contracts/telemetry-lora.md).

**Rationale**: The user already runs the `ra08h-dp-comm` gateway and wants its
telemetry path reused. P2P avoids LoRaWAN provisioning/keys/network server and is
the minimal change to get uplinks decoded by the existing gateway. dp-lumen sends
raw bytes; base64 is only the gateway's UART-side encoding.

**Alternatives considered**:
- *LoRaWAN Class A*: better for multi-gateway fleets but requires keys, join, and a
  network server; rejected — no gateway/NS available and out of scope this version.
- *New P2P framing*: rejected — would break the existing gateway decoder.

**Triggers**: on-change (mode/band/load/mains) immediate via the 50 ms loop
(satisfies SC-005 ≤ 60 s) + 15 min heartbeat (FR-019). No TX in deep sleep. Failed
TX never blocks emergency operation and does **not** reset the MCU (the gateway
bridge resets on TxDone; dp-lumen must not).

**Battery percentage**: linear `clamp_0_100((vbat_mv - 5400) * 100 / 1300)`
(5.4 V = 0 %, 6.7 V = 100 %). Pure `battery_pct_from_mv()`, host-tested. Accepted as
a lead-acid approximation for fleet monitoring.

---

## 6b. RF antenna switch GPIO

**Decision**: Drive the RA-08H module antenna switch using the reference pins from
`ra08h-dp-comm` `inc/lora_config.h`: `CONFIG_LORA_RFSW_CTRL` = `GPIOD`/`PIN_11`,
`CONFIG_LORA_RFSW_VDD` = `GPIOA`/`PIN_10`.

**Rationale**: These are module-internal antenna-switch controls used by the SX126x
board driver, not application I/O. They are not in the constitution GPIO table and do
not collide with app pins (IO4/5/7/8/9/11/14/15, IO2, RSTN). The same RA-08H module
is used by `ra08h-dp-comm`, where these pins are proven.

**Action for implement**: Confirm against the dp-lumen schematic before flashing;
keep the values in a `lora_config.h` mirrored from the reference.

---

## 6c. Build & flashing toolchain (parity with ra08h-dp-comm)

**Decision**: Build the device firmware with the **Tremo SDK Makefile** at
`sdk/ra08h-dp-lumen/` (`TREMO_SDK_PATH`, default `/sdk` in the devcontainer), adding
the LoRa SDK sources (`lora/system`, `lora/system/crypto`, `lora/radio/sx126x`,
`lora/driver`), include paths, `-DUSE_MODEM_LORA -DREGION_CN470`, and `libcrypto.a`.
Provide a `.devcontainer` (image `ra08h-env`) and `julynx.project-actions` status-bar
buttons (Build/Clean/Flash/Monitor) mirrored in `.vscode/tasks.json`.

**Rationale**: Reuses the user's preferred `ra08h-dp-comm` flashing workflow exactly.
`make flash` uses `common.mk` with `SERIAL_PORT`/`SERIAL_BAUDRATE`. The host CMake
stub + unit tests stay separate (logic/codec validation without hardware).

**Monitor baud**: 115200 (dp-lumen diag UART per `platform_uart.c`), unlike the
reference's 9600.

**Alternatives considered**:
- *Keep only the `RA08H_SDK_PATH` wrapper scripts*: works but lacks the requested
  one-click buttons and reproducible devcontainer.

---

## 7. Main Loop Architecture

**Decision**: **50 ms periodic tick** (timer interrupt or SDK soft-timer) driving:
ADC sample → domain `luminaire_sm_tick()` → GPIO/LED update → UART diag on change.

**Rationale**: Matches debounce window (50 ms); simple enough for single-core MCU;
keeps callbacks thin per AGENTS.md.

**Alternatives considered**:
- *Fully event-driven EXTI only*: misses periodic battery band evaluation and
  heartbeat timing for future P3.

---

## Resolved Items

All Technical Context items resolved for P1+P2 **and P3 (LoRa P2P) + tooling**.
No `NEEDS CLARIFICATION` remains. Open hardware confirmations (not blockers):
RF-switch pins vs schematic, final `product_id`, battery% curve vs measured SOC.
