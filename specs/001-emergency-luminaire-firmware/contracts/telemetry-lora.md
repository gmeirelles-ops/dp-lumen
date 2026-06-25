# Contract: LoRa Telemetry (Deferred)

**Feature**: `001-emergency-luminaire-firmware`  
**Status**: **DEFERRED** — not in current implementation phase (P3)  
**User story**: US3 — Telemetria LoRa para gestão de frota

## Purpose

Future uplink-only telemetry for centralized fleet monitoring. **No downlink control**
in any version covered by this spec (FR-020). Remote commands remain out of scope.

## Implementation phase

| Item | Current phase (P1+P2) | Future phase (P3) |
|---|---|---|
| Payload encode | No | Yes |
| LoRaWAN join | No | Yes |
| Uplink on change | No | Yes |
| Heartbeat 15 min | No | Yes |
| Downlink processing | No | Never (this feature) |

## Intended payload (draft v0)

JSON or compact binary TBD in P3 planning; logical fields:

| Field | Type | Description |
|---|---|---|
| `device_id` | uint32 | Provisioned per device |
| `mains_present` | bool | Grid power present |
| `battery_band` | enum | `charged`, `normal`, `low`, `critical` |
| `load_state` | enum | `on`, `off` |
| `mode` | enum | `mains`, `battery`, `sleep` |
| `vbat_mv` | uint16 | Optional raw battery mV |
| `seq` | uint16 | Monotonic sequence |

## Transmission triggers (future)

1. **On change**: `OperatingMode` or `BatteryBand` change → uplink within 60 s (SC-005).
2. **Heartbeat**: every 15 min if no change (FR-019).
3. **Failure**: LoRa unavailable → local operation continues (edge case in spec).

## Radio assumptions (future)

- Integrated RA-08H LoRa radio (not external SPI module).
- Region: Brazil ISM **915 MHz** (confirm gateway compatibility in P3 plan).
- LoRaWAN Class A uplink preferred for fleet gateways.

## Domain hook (current phase)

`luminaire_state_t` may populate `TelemetrySnapshot` in memory; no RF calls.
`connectivity/lora_telemetry` component stub optional with empty `send()`.

## Security

- Keys and DevEUI stored in protected flash / provisioning tool — never in git.
- Not logged on UART (constitution IV).
