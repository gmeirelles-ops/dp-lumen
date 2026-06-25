#include "telemetry_codec.h"

#include <stddef.h>

uint8_t checksum_calc(const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;

    if (data == NULL) {
        return 0;
    }

    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }

    sum = ~sum;
    sum++;
    return sum;
}

void crypt(uint8_t *data, uint16_t len)
{
    if (data == NULL) {
        return;
    }

    for (uint16_t i = 0; i < len; i++) {
        data[i] = (uint8_t)(~data[i]);
    }
}

size_t telemetry_encode(const telemetry_snapshot_t *snap, uint8_t *out, size_t cap)
{
    if (snap == NULL || out == NULL || cap < TELEMETRY_FRAME_LEN) {
        return 0;
    }

    uint8_t payload[TELEMETRY_PAYLOAD_LEN];
    payload[0] = snap->product_id;

    uint8_t flags = 0;
    if (snap->ac_present) {
        flags |= TELEMETRY_FLAG_AC_PRESENT;
    }
    if (snap->load_on) {
        flags |= TELEMETRY_FLAG_LOAD_ON;
    }
    payload[1] = flags;
    payload[2] = snap->battery_pct;

    out[0] = checksum_calc(payload, TELEMETRY_PAYLOAD_LEN);
    out[1] = payload[0];
    out[2] = payload[1];
    out[3] = payload[2];

    crypt(out, TELEMETRY_FRAME_LEN);
    return TELEMETRY_FRAME_LEN;
}
