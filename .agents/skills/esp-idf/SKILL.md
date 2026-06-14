---
name: esp-idf
description: Use for ESP-IDF firmware projects in C/C++ involving ESP32-family devices, FreeRTOS, Wi-Fi, BLE/Blufi provisioning, MQTT/TLS, OTA, NVS, peripheral drivers, field diagnostics, embedded Clean Code/Clean Architecture, and Spec Kit tasks. Do not use for Flutter app code, backend services, generic desktop C, or non-ESP-IDF projects.
---

# ESP-IDF Skill

You are working as a senior ESP-IDF firmware engineer for IoT devices.

Your priorities are:

1. correctness on real hardware
2. field diagnosability
3. safe connectivity behavior
4. maintainability
5. embedded Clean Code and Clean Architecture
6. compatibility with existing devices
7. clear integration with Spec Kit

## Operating mode

The frontmatter is the trigger surface. After this skill is active, use it as the ESP-IDF operating checklist for firmware tasks, connectivity, storage, drivers, FreeRTOS, diagnostics, build failures and Spec Kit implementation work.

When this skill is used inside a repository with `AGENTS.md`, treat `AGENTS.md` as the project policy and source of truth for critical restrictions. The user request defines the work objective, but it does not implicitly bypass hardware contracts, security, provisioning compatibility, MQTT/NVS/OTA/TLS contracts or destructive-operation restrictions.

For ESP-ADF audio-specific implementation, prefer the `esp-adf` skill. When an audio task touches Wi-Fi, BLE/Blufi, MQTT, TLS, OTA, NVS, FreeRTOS or app/backend contracts, apply the relevant ESP-IDF rules from this skill as well.

## First actions

Before editing code, inspect the repository.

Read the smallest set that can safely answer the task:

- project policy: `AGENTS.md`
- Spec Kit policy and active feature docs: `.specify/memory/constitution.md`, `spec.md`, `plan.md`, `tasks.md`
- build/config entry points: root `CMakeLists.txt`, `main/CMakeLists.txt`, component `CMakeLists.txt`, `sdkconfig.defaults`, `sdkconfig`, partition table files
- affected modules: MQTT, Wi-Fi, BLE/Blufi, OTA, NVS, diagnostics, drivers, public headers and component boundaries

Identify:

- ESP-IDF target
- ESP-IDF version
- hardware revision assumptions
- existing component boundaries
- current connectivity state machine
- current task model
- persistent storage schema
- external contracts with app/backend
- where business/application logic currently lives
- which modules own drivers, connectivity, protocol and storage

If multiple Spec Kit features or multiple `spec.md`/`plan.md`/`tasks.md` files exist, identify the active one from the branch, command context or user request. Ask for confirmation before implementation if it remains ambiguous.

Never start by generating new architecture without checking what already exists.

# Architecture quick rules

Apply Clean Code and Clean Architecture pragmatically. The goal is firmware that is reliable, understandable, testable and safe to evolve without breaking hardware contracts.

Keep application/domain logic independent from ESP-IDF infrastructure details. Typical boundaries are `app/domain`, `protocol`, `connectivity`, `storage`, `drivers` and `platform`; keep MQTT topics/payloads in `protocol`, connection lifecycle in `connectivity`, NVS schema in `storage`, hardware access in `drivers`, and ESP-IDF/board specifics in `platform`.

Prefer small explicit APIs, opaque handles for stateful components, private structs in `.c` files, bounded string operations, checked return values and `esp_err_t` for fallible public functions. Avoid global mutable state, long callbacks, scattered lifecycle booleans, duplicated topics/NVS keys/payload schemas and abstraction layers that only forward calls.

Use ports/adapters only when they reduce real coupling or improve tests. A small port can keep application logic independent from `esp_mqtt_client_handle_t`:

```c
typedef struct {
    esp_err_t (*publish)(void *ctx, const char *topic, const char *payload, int qos, bool retain);
    void *ctx;
} app_mqtt_port_t;
```

Prefer explicit state machines for connection, provisioning, OTA and device behavior. State transitions should be visible in code and logs; avoid hidden lifecycle encoded in unrelated booleans.

Keep pure or near-pure logic testable without hardware where practical: topic construction, payload parsing, diagnostic mapping, backoff calculation, configuration validation, state transitions, NVS migration logic and command routing.

Treat these as problems unless explicitly justified: `app_main.c` containing most of the application, callbacks doing business logic or hardware work directly, one `manager.c` owning unrelated subsystems, public headers exposing private fields, private-header coupling across components, circular dependencies, retry loops without backoff and unbounded buffers.

## Architecture review

Before final response, check:

- Is business logic separated from ESP-IDF callbacks and hardware drivers?
- Are component APIs small and clear?
- Are public headers minimal?
- Is ownership of state and memory clear?
- Are external contracts preserved?
- Are logs and diagnostics useful in the field?
- Are state transitions explicit?
- Are new abstractions justified by testability, decoupling or clarity?
- Did build pass, or is the reason for not running it clearly stated?

## Working with Spec Kit

Spec Kit is the project workflow driver.

### During `/speckit.specify`

Focus on requirements, not implementation.

Ensure the spec captures:

- device behavior
- user-visible behavior
- app/backend contract
- connectivity states
- failure modes
- hardware assumptions
- pin constraints
- provisioning requirements
- MQTT topics/payloads
- diagnostics expected by app/backend
- OTA expectations
- persistence expectations
- acceptance criteria
- hardware validation criteria
- clean architecture boundaries when relevant

Do not implement code during this phase.

### During `/speckit.plan`

Create a technical plan that includes:

- ESP-IDF components to modify/create
- proposed component boundaries
- dependency direction
- FreeRTOS tasks/events/timers
- state machines
- NVS schema changes
- MQTT topic/payload compatibility
- Wi-Fi/BLE/MQTT reconnection strategy
- error code model
- logging strategy
- OTA/partition implications
- memory/heap impact
- test strategy
- hardware validation steps

Flag risky changes before implementation.

### During `/speckit.tasks`

Tasks must be small, ordered and file-specific.

Prefer tasks that can be validated independently.

Include tasks for:

- public interfaces/headers
- domain/application logic
- adapters/drivers/connectivity/storage
- tests or test scaffolding when possible
- data structures
- implementation
- integration
- logs/diagnostics
- docs/spec updates
- build validation
- hardware validation notes

Do not create vague tasks such as “fix connectivity” or “improve driver”. Split them into concrete changes.

### During `/speckit.implement`

Follow `tasks.md`.

Do not implement unrelated improvements.

For each task:

1. read the relevant code
2. verify the intended layer/component
3. implement the smallest correct change
4. preserve existing external contracts
5. build or explain why build was not run
6. update task status only after validation

When a task is wrong or incomplete, stop and update the plan/tasks instead of improvising.

## Firmware design rules

Prefer explicit state machines over scattered boolean flags.

Avoid mixing provisioning, Wi-Fi, MQTT and business logic in the same function.

Separate concerns:

- hardware drivers
- connectivity
- storage
- protocol/payload
- application logic
- diagnostics
- OTA
- configuration

## ESP-IDF API style

Use ESP-IDF conventions:

- `esp_err_t` for fallible public functions
- `ESP_OK` on success
- `ESP_ERR_INVALID_ARG` for invalid arguments
- `ESP_ERR_INVALID_STATE` for wrong lifecycle state
- `ESP_FAIL` only when no better error exists
- `ESP_RETURN_ON_ERROR`
- `ESP_RETURN_ON_FALSE`
- `ESP_GOTO_ON_ERROR` for cleanup paths

Validate all pointers and sizes.

Avoid silent failure.

## Public APIs

For new components, prefer:

```c
typedef struct component_name_t *component_name_handle_t;
```

Public header:

- opaque handle
- config struct
- lifecycle functions
- minimal public surface

Private implementation:

- full struct in `.c`
- internal helpers as `static`
- no unnecessary global state

Example lifecycle shape:

```c
esp_err_t component_name_create(const component_name_config_t *config,
                                component_name_handle_t *out_handle);

esp_err_t component_name_start(component_name_handle_t handle);

esp_err_t component_name_stop(component_name_handle_t handle);

esp_err_t component_name_destroy(component_name_handle_t handle);
```

## FreeRTOS rules

Do not block ESP-IDF event handlers.

Do not perform heavy work in callbacks.

Use queues/event groups/timers to hand work to tasks.

When creating a task, document or make obvious:

- task name
- stack size
- priority
- core affinity, when used
- shutdown behavior
- watchdog considerations

Shared mutable state must be protected.

Use `FromISR` APIs in interrupts.

## Connectivity rules

### Wi-Fi

On disconnect, capture and log the reason code when available.

Distinguish at least:

- authentication failure
- AP not found
- association failure
- handshake timeout
- beacon timeout
- user-triggered disconnect
- unknown or unsupported reason

Use backoff for retries.

Avoid infinite tight reconnect loops.

### BLE/Blufi

Do not log Wi-Fi password or sensitive provisioning data.

Return meaningful provisioning status to the app.

Do not assume BLE and Wi-Fi timing is reliable without state handling.

### MQTT

Preserve existing topic names and payload schema unless a Spec Kit task explicitly changes them.

When publishing:

- validate connection state
- validate payload length
- handle offline case
- check return codes
- log publish failure clearly

For MQTT client errors, inspect transport/TLS/socket error info when available.

Use LWT when the existing architecture supports it or the spec requires device online/offline visibility.

## Diagnostics

Prefer structured diagnostic codes over free-form strings.

A good diagnostic includes:

- stable code
- short user-safe message
- technical detail for logs
- source subsystem
- timestamp or uptime when available
- retry count when relevant
- raw ESP-IDF reason/error when safe

Do not expose secrets in diagnostics.

## Memory and safety

Avoid unbounded copies.

Prefer `snprintf` over `sprintf`.

Check buffer sizes.

Avoid repeated heap allocation in loops.

Avoid memory ownership ambiguity.

Clearly define who owns allocated memory.

Free all resources on error paths.

## Logging

Every `.c` file with logs should define:

```c
static const char *TAG = "component";
```

Logs should answer:

- what failed
- where it failed
- why it likely failed
- whether firmware will retry
- whether user action is required

Never log secrets.

## Build and validation

After code changes, run:

```bash
idf.py build
```

When relevant, run:

```bash
idf.py size
```

Do not run destructive device operations unless the user explicitly asked.

Do not run:

```bash
idf.py erase_flash
idf.py flash
```

unless explicitly authorized.

## Output checklist

When finishing, report:

- changed files
- implementation summary
- architecture/component boundary notes
- build/test commands run
- build/test results
- remaining risks
- hardware validation still needed
- any Spec Kit files that should be updated
