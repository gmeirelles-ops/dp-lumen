# Data Model: Emergency Luminaire Firmware

**Feature**: `001-emergency-luminaire-firmware`  
**Date**: 2026-06-24

## Overview

The firmware maintains a small set of typed states derived from ADC readings and
button events. Domain logic is pure where possible (testable without hardware).

## Entities

### OperatingMode

Current high-level mode of the luminaire.

| Value | Description |
|---|---|
| `MainsPresent` | Grid power detected (`AC_DETECT` = present) |
| `BatteryEmergency` | Grid absent; battery powers lamps (default ON) |
| `DeepSleepProtection` | VBAT ≤ 5.4 V; lamps off; low-power hibernation |

### BatteryBand

Derived from `V_BATT_ADC` (millivolts at battery, post-calibration).

| Value | Condition (V_in) |
|---|---|
| `Charged` | ≥ 6.7 V |
| `Normal` | > 5.8 V and < 6.7 V |
| `Low` | ≤ 5.8 V and > 5.4 V |
| `Critical` | ≤ 5.4 V |

Hysteresis: when leaving `Low`, require > 5.9 V to avoid LED flicker (50 mV band).

### LoadState

| Value | Description |
|---|---|
| `On` | `LAMP_CTRL` active — LED1 + LED2 powered |
| `Off` | Lamps off |

### MainsState

| Value | Description |
|---|---|
| `Present` | +12V ADC above enter threshold (hysteresis) |
| `Absent` | Below exit threshold |

### TestSession

| Field | Type | Description |
|---|---|---|
| `active` | bool | True while maintenance test cycle holds lamps on after `btn.test` |
| `load_override` | LoadState | Lamp state requested by test toggle |

When `active` and mains returns, lamps stay per test session until test ends (US2
scenario 6). Clearing: mains lost, or `btn.test` toggles off, or explicit timeout
(optional — not in spec; default: until next test toggle).

### ButtonEvent

| Field | Type | Description |
|---|---|---|
| `source` | `Power` \| `Test` | Which button |
| `action` | `Press` | Debounced falling edge (active-low) |

### LuminaireState (aggregate root)

| Field | Type | Description |
|---|---|---|
| `mode` | OperatingMode | Current mode |
| `mains` | MainsState | Filtered mains detection |
| `battery` | BatteryBand | Current battery band |
| `load` | LoadState | Lamp output |
| `test_session` | TestSession | Maintenance test state |
| `led_status` | bool | Derived indicator |
| `led_battery` | bool | Derived indicator |
| `led_load` | bool | Derived indicator |

### BatteryPercent (P3)

Derived from VBAT for telemetry. Pure function
`battery_pct_from_mv(uint32_t vbat_mv) -> uint8_t`:

```text
battery_pct = clamp_0_100( (vbat_mv - 5400) * 100 / 1300 )
```

5.4 V → 0 %, 6.7 V → 100 % (linear; lead-acid approximation).

### TelemetrySnapshot (P3 — active, LoRa P2P uplink)

| Field | Type | Notes |
|---|---|---|
| `product_id` | uint8 | Provisional `4` (not final) |
| `ac_present` | bool | Mains present (status_flags bit0) |
| `load_on` | bool | Lamps on (status_flags bit1) |
| `battery_pct` | uint8 | 0–100 %, from `battery_pct_from_mv()` |

Built by `components/domain/telemetry` from `LuminaireState`. A change-detection
helper compares the new snapshot's trigger fields (mode/band/load/mains) against the
last sent snapshot to decide on-change transmission.

#### On-air frame (see contracts/telemetry-lora.md)

```text
payload[3] = { product_id, status_flags, battery_pct }   // status_flags: b0=AC, b1=load
checksum   = ~(sum(payload)) + 1                          // checksum_calc
frame[4]   = crypt( [checksum, payload...] )              // crypt = bitwise NOT each byte
Radio.Send(frame, 4)                                      // wire-compatible with gateway
```

`telemetry_encode()` + `checksum_calc()` + `crypt()` live in
`components/protocol/telemetry_codec` (pure, host-tested). `connectivity/lora_radio`
sends; no domain coupling to the radio SDK.

## State Transitions

```text
                    AC lost (stable 200ms)
    MainsPresent ---------------------------> BatteryEmergency
         ^    |                                      |
         |    | AC restored (stable)                  | VBAT <= Critical
         |    v (unless TestSession holds load)       v
         +-------------------------------- DeepSleepProtection
                    AC restored OR VBAT > wake
```

### Transition table

| From | Event | To | Side effects |
|---|---|---|---|
| `MainsPresent` | Mains absent (stable) | `BatteryEmergency` | `load` → On; `led.status` off; `led.load` off |
| `BatteryEmergency` | Mains present (stable) | `MainsPresent` | `load` → Off*; `led.status` on |
| `BatteryEmergency` | VBAT ≤ 5.4 V | `DeepSleepProtection` | `load` → Off; enter sleep |
| `DeepSleepProtection` | Mains present | `MainsPresent` | Wake; normal mains rules |
| `DeepSleepProtection` | VBAT > 5.6 V (no mains) | `BatteryEmergency` | Wake; `load` → On |
| Any (not DeepSleep) | `btn.test` press (mains only) | — | Toggle `load`; set `TestSession` |
| `BatteryEmergency` | `btn.power` press | — | Toggle `load` |

\*If `TestSession.active` and test left lamps on, apply US2 scenario 6 rules.

## Validation Rules

- `btn.test` MUST NOT produce `ButtonEvent` when `mains` = Absent.
- `btn.power` MUST NOT produce `ButtonEvent` when `mains` = Present or
  `mode` = `DeepSleepProtection`.
- `led.load` MUST be false when `mode` ≠ `MainsPresent`.
- `led.status` MUST equal (`mains` = Present).
- `led.battery` MUST be true when `battery` ∈ {`Low`, `Critical`} and mode ≠
  `DeepSleepProtection` (indicators off in deep sleep allowed).

## Domain API (conceptual)

```c
void luminaire_sm_init(luminaire_state_t *st);
void luminaire_sm_tick(luminaire_state_t *st, const luminaire_inputs_t *in);
// inputs: mains_present, vbat_mv, btn_power, btn_test
```

`battery_band_from_mv(uint32_t vbat_mv)` — pure function, unit-tested.
`battery_pct_from_mv(uint32_t vbat_mv)` — pure function, unit-tested (P3 telemetry).

```c
// P3 telemetry (pure, host-tested)
size_t telemetry_encode(const telemetry_snapshot_t *s, uint8_t *out, size_t cap);
uint8_t checksum_calc(const uint8_t *data, uint8_t len);
void    crypt(uint8_t *data, uint16_t len);
```
