# Quickstart: Emergency Luminaire Firmware Validation

**Feature**: `001-emergency-luminaire-firmware`  
**Scope**: P1 + P2 + UART diagnostics + **P3 LoRa P2P uplink**

## Prerequisites

- dp-lumen board (KiCad v0.0.1) with RA-08H programmed with firmware under test
- Adjustable DC supply for +VBAT (4.5 V – 7.0 V) on J3
- +12 V source to simulate mains rail (or AC input J1 with grid power)
- USB-UART adapter on J2 (115200 8N1)
- Multimeter for VBAT verification
- For QS-09: a `ra08h-dp-comm` gateway powered and connected to its host UART
  (915 MHz P2P), to receive dp-lumen uplinks

## Build / Flash / Monitor (status-bar buttons)

The devcontainer (`ra08h-env`, SDK at `/sdk`) provides `julynx.project-actions`
status-bar buttons that run in `sdk/ra08h-dp-lumen`:

- **🔨 Build** → `make`
- **🧹 Clean** → `make clean`
- **🚀 Flash** → `make flash` (board in BOOT/download mode; `SERIAL_PORT`/`SERIAL_BAUDRATE`)
- **📺 Monitor** → `python3 -m serial.tools.miniterm /dev/ttyUSB0 115200`

The same actions exist in `.vscode/tasks.json`. Host logic/unit-test build stays
separate (`cmake -B build -S .` / `ctest`).

Flash only when explicitly authorized (requires user approval per AGENTS.md).

## Bench scenarios

### QS-01 — Mains present, idle (SC-001)

1. Apply +12 V mains sense (or AC on J1).
2. Set VBAT to 6.8 V.
3. **Expect**: `led.status` ON; lamps OFF; `led.load` ON; `led.battery` OFF.
4. UART: `mode=mains mains=1 batt=charged load=off`

### QS-02 — Test button toggle (SC-003, mains)

1. With mains present, press `btn.test`.
2. **Expect**: lamps ON.
3. Press again → lamps OFF.
4. Press `btn.power` → no effect; UART `btn=ignored:power`.

### QS-03 — Grid loss, auto emergency (SC-002)

1. Start with mains on, lamps off.
2. Remove mains within 1 s measurement window.
3. **Expect**: lamps ON without button; `led.status` OFF within 1 s.

### QS-04 — Power button on battery (SC-003)

1. Mains off, VBAT 6.5 V, lamps on (auto).
2. Press `btn.power` → lamps OFF.
3. Press again → lamps ON.
4. Press `btn.test` → no effect; UART `btn=ignored:test`.

### QS-05 — Low battery indicator

1. Mains off or on.
2. Lower VBAT to 5.7 V.
3. **Expect**: `led.battery` ON.

### QS-06 — Charged indicator (mains only)

1. Mains on, VBAT 6.8 V.
2. **Expect**: `led.load` ON.
3. Remove mains.
4. **Expect**: `led.load` OFF (economy).

### QS-07 — Deep sleep and wake

1. Mains off, lower VBAT to 5.3 V.
2. **Expect**: lamps OFF; low current (sleep); UART silent.
3. **Wake A**: apply mains → normal mains mode (QS-01).
4. **Wake B** (separate run): raise VBAT to 5.7 V without mains → battery
   emergency, lamps ON.

### QS-08 — UART diagnostics (FR-021)

1. Run QS-03 while capturing serial.
2. **Expect**: line with `mode=battery` on transition; no secrets in output.

### QS-09 — LoRa P2P uplink reception (FR-017, FR-018, SC-005)

1. Power the `ra08h-dp-comm` gateway; capture its host UART output.
2. Boot dp-lumen with mains present, VBAT 6.8 V.
3. **Expect**: gateway prints a frame whose decoded payload (after un-`crypt` +
   checksum OK) is `product_id=4`, AC present (status bit0=1), `battery_pct ≈ 100`,
   load bit reflecting lamp state.
4. Remove mains → lamps auto-on.
5. **Expect**: within ≤ 60 s, gateway prints a new frame with AC absent (bit0=0),
   load bit set, `battery_pct` per VBAT.
6. Leave state unchanged for > 15 min.
7. **Expect**: a heartbeat frame arrives (FR-019).
8. Enter deep sleep (QS-07).
9. **Expect**: no frames transmitted while in `DeepSleepProtection`.

## Pass criteria

All scenarios QS-01 through QS-09 pass on hardware. Unit tests for domain and
`telemetry_codec` pass on host:

```bash
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

## Out of scope for this quickstart

- Remote command downlink tests (SC-006 — radio is TX-only; downlink excluded, FR-020)
- LoRaWAN join / network-server validation (not used; P2P only)

## References

- [data-model.md](./data-model.md) — states and transitions
- [contracts/uart-diagnostics.md](./contracts/uart-diagnostics.md) — log format
- [research.md](./research.md) — ADC thresholds
