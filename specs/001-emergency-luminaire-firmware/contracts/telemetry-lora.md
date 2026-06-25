# Contract: LoRa Telemetry (Active — LoRa P2P uplink)

**Feature**: `001-emergency-luminaire-firmware`  
**Status**: **ACTIVE (P3)** — implemented in this phase  
**User story**: US3 — Telemetria LoRa para gestão de frota  
**Supersedes**: the previous LoRaWAN draft (v0) of this contract.

## Purpose

Uplink-only telemetry for centralized fleet monitoring over **raw LoRa P2P**
(no LoRaWAN join, no network server). The transmission is **wire-compatible with
the `ra08h-dp-comm` gateway** (`git@github.com:diponto/ra08h-dp-comm.git`), which
receives the LoRa frame and forwards it (base64 + RSSI) over its UART to the
backend.

**No downlink control** in any version covered by this spec (FR-020). Any received
radio frame is ignored; the dp-lumen radio is configured TX-only.

## Radio parameters (must match the gateway)

Mirror of `ra08h-dp-comm` `src/app.c`:

| Parameter | Value |
|---|---|
| Frequency | 915 MHz (`SetChannel(915000000)`) |
| Modem | `MODEM_LORA` |
| TX power | 22 dBm |
| Bandwidth | 500 kHz (`LORA_BANDWIDTH = 2`) |
| Spreading factor | SF7 |
| Coding rate | 4/5 (`LORA_CODINGRATE = 1`) |
| Preamble length | 8 |
| CRC | on |
| Fixed-length payload | off |
| IQ inversion | off |
| TX timeout | 3000 ms |

Region build define mirrors the reference (`-DUSE_MODEM_LORA -DREGION_CN470`);
the channel is set explicitly to 915 MHz, so the region table is not used for P2P.

## RF antenna switch (module-internal)

From `ra08h-dp-comm` `inc/lora_config.h` (RA-08H module reference design):

| Macro | Port/Pin |
|---|---|
| `CONFIG_LORA_RFSW_CTRL_GPIOX` / `_PIN` | `GPIOD` / `GPIO_PIN_11` |
| `CONFIG_LORA_RFSW_VDD_GPIOX` / `_PIN` | `GPIOA` / `GPIO_PIN_10` |

These are RA-08H module-internal antenna-switch controls, not application I/O, and
do not collide with the constitution GPIO table (app pins IO4/5/7/8/9/11/14/15,
IO2, RSTN). Confirm against the dp-lumen schematic during `/speckit-implement`.

## Application payload (logical)

Fixed 3-byte payload (`PAYLOAD_LEN = 3`):

| Offset | Field | Type | Description |
|---|---|---|---|
| 0 | `product_id` | uint8 | Product type id. **Provisional `4`** (not final). |
| 1 | `status_flags` | uint8 | bit0 = AC present (1=mains), bit1 = load ON (1=lamps on). bits 2-7 reserved = 0 |
| 2 | `battery_pct` | uint8 | Battery charge 0–100 %, derived from VBAT (see mapping) |

There is **no per-device unique id** in this version (risk: multiple devices with
the same `product_id` are indistinguishable at the gateway). Reserved for a future
contract revision (e.g. ASR6601 chip id).

### Battery percentage mapping

Linear approximation, clamped to `[0, 100]`:

```text
battery_pct = round( (vbat_mv - 5400) * 100 / (6700 - 5400) )
            = clamp_0_100( (vbat_mv - 5400) * 100 / 1300 )
```

- 5.4 V (critical / FR-014) → 0 %
- 6.7 V (charged / FR-007) → 100 %

This is an approximation for a 6 V lead-acid cell (true SOC curve is non-linear);
accepted for fleet-level monitoring.

## On-air frame (must match the gateway)

The gateway expects: `OTA = crypt( [checksum] ++ payload )`.

1. `checksum = checksum_calc(payload, PAYLOAD_LEN)` — two's complement of the byte
   sum: `cs = ~(sum) + 1`.
2. `frame[0] = checksum`, `frame[1..PAYLOAD_LEN] = payload`.
3. `crypt(frame, PAYLOAD_LEN + 1)` — bitwise NOT of **every** byte (checksum
   included).
4. `Radio.Send(frame, PAYLOAD_LEN + 1)` — total on-air length = 4 bytes.

The gateway un-`crypt`s, verifies `checksum`, appends its measured RSSI byte to the
payload, base64-encodes, and prints over its UART. dp-lumen does **not** base64 —
base64 is only the gateway's UART-side encoding.

> Compatibility note: `checksum_calc` and `crypt` are copied verbatim from the
> gateway so frames decode correctly. Do not "improve" them without a coordinated
> gateway change.

## Transmission triggers

1. **On change**: when `OperatingMode`, `BatteryBand`, `LoadState`, or mains state
   changes → uplink immediately (50 ms loop ⇒ well within SC-005 ≤ 60 s).
2. **Heartbeat**: every **15 min** when no change (FR-019).
3. **Deep sleep**: no transmission while in `DeepSleepProtection` (radio idle).
4. **Failure**: a failed/timed-out TX MUST NOT block local emergency operation;
   the next trigger retries. Unlike the gateway bridge, dp-lumen does **not**
   `NVIC_SystemReset` on TxDone/timeout.

## Architecture mapping

- `components/domain` — builds `telemetry_snapshot_t` from `luminaire_state_t`;
  `battery_pct_from_mv()` is pure and host-tested. No radio headers.
- `components/protocol/telemetry_codec` — pure `telemetry_encode()`, `checksum_calc()`,
  `crypt()`; host-tested; no radio headers.
- `components/connectivity/lora_radio` — SX126x init, `SetTxConfig`,
  `SetChannel`, RF-switch GPIO, `Radio.Send`, `Radio.IrqProcess()` pump.

## Security

- No keys/credentials are used by P2P; none are committed.
- `crypt` (bitwise NOT) is **obfuscation only**, not encryption — do not treat the
  link as confidential.
- RF parameters and any future keys MUST NOT appear on the UART diag (constitution IV).
