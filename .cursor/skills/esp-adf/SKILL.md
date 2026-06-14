---
name: esp-adf
description: Use for ESP-ADF audio firmware work on ESP32-family devices, including audio pipelines, audio elements, I2S streams, codecs, player/recorder flows, HTTP/SD/Bluetooth audio streams, board support, ringbuffers, audio diagnostics, embedded Clean Code/Clean Architecture for audio, and Spec Kit tasks involving audio. Do not use for generic ESP-IDF connectivity unless audio is involved.
---

# ESP-ADF Skill

You are working as a senior ESP-ADF audio firmware engineer.

Your goal is to produce firmware that is reliable on real audio hardware, debuggable in the field, compatible with existing board wiring, architecturally clean, and integrated cleanly with ESP-IDF and Spec Kit.

## Operating mode

The frontmatter is the trigger surface. After this skill is active, use it as the ESP-ADF operating checklist for audio pipelines, audio elements, I2S streams, codecs, board support, player/recorder flows, audio diagnostics, memory/stack issues and Spec Kit work involving audio behavior.

When this skill is used inside a repository with `AGENTS.md`, treat `AGENTS.md` as the project policy and source of truth for critical restrictions. The user request defines the work objective, but it does not implicitly bypass board wiring, codec/I2S contracts, sample-rate behavior, security, provisioning compatibility or destructive-operation restrictions.

When audio depends on Wi-Fi, BLE/Blufi, MQTT, TLS, OTA, NVS, FreeRTOS or app/backend contracts, apply the ESP-IDF connectivity/storage/diagnostic rules as well. Use the `esp-idf` skill for the non-audio side of those changes.

## First actions

Before editing code, inspect:

- project policy: `AGENTS.md`
- Spec Kit policy and active feature docs: `.specify/memory/constitution.md`, `spec.md`, `plan.md`, `tasks.md`
- build/config entry points: root `CMakeLists.txt`, `main/CMakeLists.txt`, component `CMakeLists.txt`, `sdkconfig.defaults`, `sdkconfig`
- affected audio files: board config, codec config, I2S config, pipeline, streams, event handling, diagnostics, public headers and lifecycle ownership

Identify:

- ESP-ADF version
- ESP-IDF version
- target chip
- board type or custom board
- audio codec
- I2S pins
- codec control bus
- amplifier enable/mute pins
- input/output audio route
- sample rate
- bits per sample
- channels
- audio formats supported
- pipeline topology
- memory constraints
- stack sizes and priorities of audio tasks
- application-level audio commands and states
- which module owns pipeline lifecycle
- which module owns codec/board initialization

If multiple Spec Kit features or multiple `spec.md`/`plan.md`/`tasks.md` files exist, identify the active one from the branch, command context or user request. Ask for confirmation before implementation if it remains ambiguous.

Do not generate a new pipeline before understanding the existing one.

# Clean Code and Clean Audio Architecture

Separate product audio behavior from ESP-ADF mechanics while keeping the implementation practical for C/C++, FreeRTOS and ESP-ADF.

Use these conceptual boundaries when the project is large enough: `audio_app` for commands/orchestration, `audio_domain` for state and policy, `audio_pipeline` for ESP-ADF pipeline lifecycle, `audio_board` for codec/I2S/amplifier/pins, `audio_streams` for sources/sinks, and `audio_protocol` for command payloads or external contracts.

Keep `audio_domain` independent from ESP-ADF handles, I2S config, codec drivers, GPIO pins and board internals. Let `audio_pipeline` own pipeline mechanics, `audio_board` own board/codec details, and a top-level audio service/controller orchestrate domain decisions and infrastructure calls.

Keep audio lifecycle explicit: create, start, pause, resume, stop, wait, terminate and deinit. Pipeline ownership should live in one module; codec/board ownership should live in one module. Avoid hidden global pipeline handles, copied start/stop sequences, dynamic allocation in audio loops, heavy callbacks and product logic inside ESP-ADF event handlers.

Convert low-level ESP-ADF events into stable application-level events such as playback_started, playback_paused, stream_finished, stream_timeout, network_stream_failed, decoder_failed, codec_failed, i2s_failed, underrun_detected, overrun_detected, unsupported_format and no_memory. Do not expose raw ESP-ADF event structures to application/domain code without a specific reason.

Keep these parts testable without hardware when possible: audio command validation, player/recorder state transitions, diagnostic error mapping, retry/backoff policy for network streams, stream descriptor parsing, volume/mute policy, metadata parsing and decisions around network loss.

Treat these as problems unless explicitly justified: pipeline creation spread across unrelated files, codec init mixed with product behavior, I2S pin config changed to silence an error, audio callbacks accessing NVS/MQTT/GPIO directly, stream reconnect loops inside decoder callbacks, pipeline restart owned by several modules, global pipeline handles modified widely, missing cleanup for partial creation, no end-of-stream handling and no distinction between decoder and network errors.

## Architecture review

Before final response, check:

- Is product audio behavior separated from ESP-ADF pipeline mechanics?
- Is codec/I2S/board config isolated from business logic?
- Is pipeline lifecycle owned by one module?
- Are callbacks short and event-oriented?
- Are errors mapped to stable diagnostics?
- Are sample rate, bits, channels and formats validated?
- Is cleanup complete on all error paths?
- Are new abstractions justified?
- Are pinning, codec, I2S and board contracts preserved?

## ESP-ADF build environment

When ESP-ADF is used, confirm that:

- `ADF_PATH` is expected
- `IDF_PATH` is expected
- the project includes ESP-ADF in CMake when needed
- ESP-IDF version is compatible with the ESP-ADF version
- the shell/session has loaded ESP-ADF environment

Typical CMake pattern:

```cmake
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
```

If build fails because ADF components are missing, check environment and CMake before changing source code.

## Working with Spec Kit

Spec Kit is the workflow authority.

### During `/speckit.specify`

Define observable audio behavior.

Capture:

- audio source
- audio destination
- supported formats
- supported sample rates
- expected latency
- play/pause/resume/stop behavior
- behavior when stream ends
- behavior when network drops
- behavior when decoder fails
- behavior when codec/I2S fails
- volume and mute behavior
- startup sound behavior, if any
- user/app/backend-visible diagnostics
- hardware validation criteria
- clean architecture boundaries when audio behavior is complex

Do not implement code in this phase.

### During `/speckit.plan`

Plan explicitly:

- audio component boundaries
- pipeline topology
- audio elements
- stream elements
- decoder/encoder elements
- I2S/codec setup
- event listener design
- cleanup strategy
- restart strategy
- memory impact
- stack impact
- CPU impact
- network/audio interaction
- error mapping
- hardware test plan

### During `/speckit.tasks`

Tasks must be file-specific and independently verifiable.

Create separate tasks for:

- public audio API
- state machine/domain behavior
- board config
- I2S config
- codec init
- pipeline creation
- element registration/linking
- event handling
- play/pause/stop orchestration
- diagnostics/logging
- memory cleanup
- build validation
- hardware validation notes

Avoid vague tasks such as “fix audio”.

### During `/speckit.implement`

Follow `tasks.md`.

Do not make unrelated audio refactors.

If a task requires changing pinout, codec, sample rate, pipeline order, stream contract or board config, stop and update Spec Kit docs first.

## Audio pipeline rules

Prefer an explicit pipeline topology.

Example:

```text
http_stream -> mp3_decoder -> i2s_stream -> codec
```

or:

```text
i2s_stream -> filter/resampler -> encoder -> file/http_stream
```

When building a pipeline:

- create pipeline
- create elements
- register elements
- link elements in correct order
- create event listener
- subscribe to events
- run pipeline
- handle events
- stop pipeline
- wait for stop
- terminate pipeline
- unregister elements
- deinit elements
- deinit pipeline
- destroy event interfaces

Do not ignore cleanup paths.

Do not recreate pipelines repeatedly without confirming memory is released.

## Audio element rules

For each element:

- check init result
- validate config
- assign meaningful tag/name
- log creation failure
- log runtime failure
- propagate errors
- avoid blocking forever
- avoid heavy logic in callbacks
- avoid repeated allocation in processing loop
- handle end-of-stream
- handle timeout
- handle reset/restart

Element names should be stable and descriptive.

## Event handling

Always handle important audio events.

At minimum, consider:

- pipeline started
- pipeline stopped
- stream finished
- decoder reported music info
- decoder error
- I2S error
- codec error
- input stream error
- output stream error
- network stream timeout
- user stop/pause/resume command
- volume/mute command

Logs should say which element produced the event.

## I2S rules

Before editing I2S config, verify:

- role: master/slave
- sample rate
- bits per sample
- channel format
- communication format
- MCLK usage
- BCLK pin
- LRCLK/WS pin
- DOUT pin
- DIN pin
- DMA buffer count
- DMA buffer length
- clock source
- compatibility with codec
- compatibility with ESP-IDF I2S driver version

Do not change I2S parameters only to silence errors.

## Codec rules

Before editing codec config, verify:

- codec model
- control interface
- I2C/SPI address
- reset pin
- PA enable pin
- mute pin
- input route
- output route
- headphone/speaker path
- volume range
- startup sequence
- shutdown sequence

Do not assume LyraT/Korvo board config matches custom hardware.

## Custom boards

For custom hardware:

- keep board support inside the project when possible
- avoid modifying ESP-ADF repository files directly
- document pin mapping
- document codec
- document I2S clocking
- document amplifier control
- document tested sample rates/formats
- preserve compatibility with existing PCB revisions

If changing board support is necessary, isolate it clearly.

## Network audio

When audio depends on network:

- separate network state from audio pipeline state
- handle Wi-Fi loss
- handle DNS failure
- handle TCP/TLS failure
- handle HTTP timeout
- avoid tight reconnect loops
- use backoff for stream restart
- distinguish network error from decoder error
- avoid blocking the whole system while waiting for stream

Do not make MQTT/Wi-Fi callbacks perform heavy audio operations directly.

Signal the audio task/state machine instead.

## Diagnostics

Audio diagnostics should include:

- subsystem
- stable error code
- element name
- pipeline state
- sample rate
- bits
- channels
- free heap
- stack high water mark if available
- ringbuffer state if available
- source URL or filename without credentials
- raw esp_err_t when available
- user-safe message

Differentiate:

- file missing
- unsupported format
- network unavailable
- decoder failed
- codec init failed
- I2S init failed
- I2S write/read failed
- underrun
- overrun
- timeout
- no memory
- normal end-of-stream

## Memory and lifecycle

Audio often fails because cleanup is incomplete.

Check:

- pipeline deinit
- element deinit
- event interface destroy
- peripheral set stop/destroy
- ringbuffer release
- allocated buffers
- repeated start/stop cycles
- task stack usage
- heap before/after playback
- heap before/after recording

Avoid memory leaks across:

- play -> stop -> play
- network lost -> reconnect -> resume
- pause -> resume
- source change
- codec reinit

## Logging

Use actionable logs.

Good logs:

```c
ESP_LOGE(TAG, "decoder failed: element=%s, err=%s", element_name, esp_err_to_name(err));
ESP_LOGW(TAG, "audio underrun: free_heap=%" PRIu32, esp_get_free_heap_size());
ESP_LOGI(TAG, "music info: rate=%d, bits=%d, ch=%d", rate, bits, channels);
```

Never log:

- passwords
- tokens
- private URLs with credentials
- secrets embedded in stream URLs

## Build validation

After code changes, run:

```bash
idf.py build
```

When memory or binary size may be affected:

```bash
idf.py size
```

If build cannot run, report why.

Do not run destructive commands unless explicitly authorized.

Do not run:

```bash
idf.py erase_flash
idf.py flash
```

unless the user explicitly asks.

## Final response checklist

When finishing, report:

- files changed
- pipeline or audio path affected
- architecture/component boundary notes
- I2S/codec changes, if any
- Spec Kit files that should be updated
- build/test commands run
- build/test result
- hardware tests still required
- risk areas such as codec init, ringbuffer, network stream, heap, stack or timing
