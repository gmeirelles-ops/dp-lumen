#ifndef TELEMETRY_CODEC_H
#define TELEMETRY_CODEC_H

#include <stddef.h>
#include <stdint.h>

#include "telemetry.h"

/* On-air format — must stay byte-compatible with the ra08h-dp-comm gateway.
 * See contracts/telemetry-lora.md. */
#define TELEMETRY_PAYLOAD_LEN 3
#define TELEMETRY_FRAME_LEN   (TELEMETRY_PAYLOAD_LEN + 1) /* checksum + payload */

#define TELEMETRY_FLAG_AC_PRESENT 0x01u
#define TELEMETRY_FLAG_LOAD_ON    0x02u

/* Build the 4-byte on-air frame: crypt([checksum, product_id, flags, battery_pct]).
 * Returns the frame length (TELEMETRY_FRAME_LEN) or 0 on invalid args. */
size_t telemetry_encode(const telemetry_snapshot_t *snap, uint8_t *out, size_t cap);

/* Two's-complement checksum (matches the gateway's checksum_calc). */
uint8_t checksum_calc(const uint8_t *data, uint8_t len);

/* In-place bitwise-NOT obfuscation (matches the gateway's crypt). Not encryption. */
void crypt(uint8_t *data, uint16_t len);

#endif
