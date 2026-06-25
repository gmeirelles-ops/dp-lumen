#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Raw LoRa P2P transmit, wire-compatible with the ra08h-dp-comm gateway.
 * Uplink only (no RX/downlink, FR-020). See contracts/telemetry-lora.md. */

void lora_radio_init(void);

/* Queue a frame for transmission. Returns false if a TX is in progress or
 * the arguments are invalid; the caller may retry on the next trigger. */
bool lora_radio_send(const uint8_t *frame, size_t len);

/* Pump radio IRQ processing; call every main-loop tick. */
void lora_radio_process(void);

/* True while a transmission is in flight. */
bool lora_radio_busy(void);

#endif
