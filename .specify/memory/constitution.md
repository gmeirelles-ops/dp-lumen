# ESP32 Firmware Template Constitution

## Core Principles

### I. Hardware Contracts Are Stable

GPIO pinout, electrical function, bootstrapping constraints, bus frequency, partition layout, NVS schema, OTA strategy, TLS configuration, codec selection, I2S pinout, board config and audio pipeline topology are compatibility contracts. They must not change without explicit requirement, documented rationale, migration impact and validation plan.

### II. Spec First, Code Second

Feature work follows the Spec Kit flow: constitution, specify, clarify when useful, checklist, plan, tasks and implement. Specification, planning and task-generation phases must not implement code. Implementation must follow approved `spec.md`, `plan.md` and `tasks.md`; if implementation reveals a requirement, architecture or hardware-contract change, update the Spec Kit artifacts before continuing.

### III. Field Diagnosability Is Required

Firmware must produce actionable diagnostics for real hardware and field support. Connectivity, storage, OTA, peripheral and audio failures must expose stable codes or structured states when practical, preserve technical causes in logs and avoid leaking secrets. Unknown errors are acceptable only after available subsystem-specific causes have been checked.

### IV. Embedded Clean Architecture, Pragmatically Applied

Application/domain logic must remain separated from ESP-IDF/ESP-ADF infrastructure details where practical. Protocol owns payloads and topics, connectivity owns Wi-Fi/BLE/MQTT/TLS lifecycle, storage owns NVS compatibility, drivers own hardware access, audio pipeline owns ESP-ADF mechanics, and board/platform modules own target-specific details. Abstractions must reduce real coupling, improve tests or clarify lifecycle.

### V. Validation Before Done

Every code change must be validated with the strongest practical check available. `idf.py build` is the default gate for firmware changes. `idf.py size` is required when firmware size, memory, partition, OTA or significant audio behavior may be affected. If hardware behavior changes, define hardware validation steps and report what still requires real-device testing.

## Template Constraints

This repository is a template for ESP32-family firmware projects. Project-specific forks must review and set:

- ESP-IDF version and target chip
- optional ESP-ADF version and `ADF_PATH` usage
- board revision and GPIO pinout
- partition table and OTA strategy
- NVS namespaces, keys and migration policy
- Wi-Fi/BLE/Blufi provisioning behavior
- MQTT topics, payload schemas, QoS, retain, LWT and keepalive
- TLS certificate, key and credential strategy
- codec, I2S, amplifier, mute/reset pins and audio sample defaults when audio is used
- field diagnostics expected by app/backend

Secrets, credentials, private keys, private certificates and production tokens must never be committed to the template or feature specs.

## Cross-Platform Development

The template supports Windows, Linux and macOS development. Spec Kit scripts are versioned in both `.specify/scripts/bash/` and `.specify/scripts/powershell/`.

- Use Bash scripts on Linux, macOS, WSL or Git Bash.
- Use PowerShell scripts on native Windows.
- Keep generated feature specs and tasks portable: avoid OS-specific paths unless the feature explicitly targets that OS.

## Governance

This constitution supersedes informal project habits and applies to every project derived from this template until explicitly amended. Amendments require a documented reason, affected compatibility contracts, migration impact and validation plan. `AGENTS.md` may add operational guidance, but it must not weaken these principles.

**Version**: 1.0.0 | **Ratified**: 2026-06-14 | **Last Amended**: 2026-06-14
