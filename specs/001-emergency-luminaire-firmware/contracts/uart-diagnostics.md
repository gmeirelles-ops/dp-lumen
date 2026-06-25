# Contract: UART Diagnostics

**Feature**: `001-emergency-luminaire-firmware`  
**Status**: Active (P1+P2)  
**Interface**: J2 programming header — UART TX from module  
**Baud**: 115200 8N1 (default; configurable in platform layer)

## Purpose

Field support reads structured single-line logs without exposing LoRa keys or RF
parameters (constitution IV).

## Log format

One line per state change or debounced button event:

```text
DP-LUMEN mode=<mode> mains=<0|1> batt=<band> load=<on|off> led=<status,batt,load> btn=<none|power|test|ignored:power|ignored:test>
```

### Field values

| Field | Values |
|---|---|
| `mode` | `mains`, `battery`, `sleep` |
| `mains` | `0` absent, `1` present |
| `batt` | `charged`, `normal`, `low`, `critical` |
| `load` | `on`, `off` |
| `led` | three chars `SBL` — `1`/`0` for status, battery, load LEDs |
| `btn` | see table below |

### `btn` values

| Value | Meaning |
|---|---|
| `none` | Tick with no button activity |
| `power` | `btn.power` accepted (battery mode) |
| `test` | `btn.test` accepted (mains mode) |
| `ignored:power` | Press during mains mode |
| `ignored:test` | Press during battery mode |

## Examples

```text
DP-LUMEN mode=mains mains=1 batt=charged load=off led=100 btn=none
DP-LUMEN mode=mains mains=1 batt=normal load=on led=100 btn=test
DP-LUMEN mode=battery mains=0 batt=low load=on led=010 btn=none
DP-LUMEN mode=battery mains=0 batt=low load=off led=010 btn=power
DP-LUMEN mode=sleep mains=0 batt=critical load=off led=000 btn=none
```

## Rules

- Emit on: mode change, battery band change, load change, button event.
- MUST NOT log credentials, LoRa keys, DevEUI, or join payloads.
- Rate limit: max 10 lines/s (burst protection).

## Consumer

Human technician via USB-UART adapter on J2 during bench and field diagnosis.
