# Quickstart: Emergency Luminaire Firmware Validation

**Feature**: `001-emergency-luminaire-firmware`  
**Scope**: P1 + P2 + UART diagnostics only (no LoRa gateway required)

## Prerequisites

- dp-lumen board (KiCad v0.0.1) with RA-08H programmed with firmware under test
- Adjustable DC supply for +VBAT (4.5 V – 7.0 V) on J3
- +12 V source to simulate mains rail (or AC input J1 with grid power)
- USB-UART adapter on J2 (115200 8N1)
- Multimeter for VBAT verification

## Build (after `/speckit-implement`)

```bash
# From repo root — exact command defined in implement phase (RA-08H SDK)
# Example placeholder:
# source $RA08H_SDK/export.sh && make -C build
```

Flash only when explicitly authorized (`idf.py flash` / RA-08H equivalent — requires
user approval per AGENTS.md).

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

## Pass criteria

All scenarios QS-01 through QS-08 pass on hardware. Unit tests for domain pass
on host:

```bash
# After implement — exact runner TBD
# ctest or make test_unit
```

## Out of scope for this quickstart

- LoRa gateway reception (P3)
- Remote command downlink tests (SC-006 — N/A until P3 commands, which are excluded)

## References

- [data-model.md](./data-model.md) — states and transitions
- [contracts/uart-diagnostics.md](./contracts/uart-diagnostics.md) — log format
- [research.md](./research.md) — ADC thresholds
