#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "telemetry.h"
#include "telemetry_codec.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while (0)

/* Decode like the ra08h-dp-comm gateway: un-crypt then verify checksum. */
static void assert_gateway_decodes(const uint8_t *frame, size_t len,
                                   uint8_t exp_pid, uint8_t exp_flags, uint8_t exp_pct)
{
    ASSERT(len == TELEMETRY_FRAME_LEN);

    uint8_t buf[TELEMETRY_FRAME_LEN];
    memcpy(buf, frame, len);
    crypt(buf, (uint16_t)len); /* crypt is its own inverse (bitwise NOT) */

    uint8_t cs = checksum_calc(&buf[1], TELEMETRY_PAYLOAD_LEN);
    ASSERT(cs == buf[0]);
    ASSERT(buf[1] == exp_pid);
    ASSERT(buf[2] == exp_flags);
    ASSERT(buf[3] == exp_pct);
}

int main(void)
{
    luminaire_state_t st;

    /* Case 1: mains present, load off, full battery. */
    memset(&st, 0, sizeof(st));
    st.mode = OPERATING_MODE_MAINS_PRESENT;
    st.mains = MAINS_STATE_PRESENT;
    st.load = LOAD_STATE_OFF;

    telemetry_snapshot_t snap;
    telemetry_snapshot_build(&snap, &st, 6700);
    ASSERT(snap.product_id == 4);
    ASSERT(snap.ac_present == true);
    ASSERT(snap.load_on == false);
    ASSERT(snap.battery_pct == 100);

    uint8_t frame[TELEMETRY_FRAME_LEN];
    size_t n = telemetry_encode(&snap, frame, sizeof(frame));
    assert_gateway_decodes(frame, n, 4, TELEMETRY_FLAG_AC_PRESENT, 100);

    /* Lock the exact wire bytes for this case (gateway compatibility). */
    ASSERT(frame[0] == (uint8_t)~0x97);
    ASSERT(frame[1] == (uint8_t)~0x04);
    ASSERT(frame[2] == (uint8_t)~0x01);
    ASSERT(frame[3] == (uint8_t)~0x64);

    /* Case 2: battery mode, load on, ~50%. */
    memset(&st, 0, sizeof(st));
    st.mode = OPERATING_MODE_BATTERY_EMERGENCY;
    st.mains = MAINS_STATE_ABSENT;
    st.load = LOAD_STATE_ON;
    telemetry_snapshot_build(&snap, &st, 6050);
    ASSERT(snap.ac_present == false);
    ASSERT(snap.load_on == true);
    ASSERT(snap.battery_pct == 50);
    n = telemetry_encode(&snap, frame, sizeof(frame));
    assert_gateway_decodes(frame, n, 4, TELEMETRY_FLAG_LOAD_ON, 50);

    /* Case 3: both flags set. */
    memset(&st, 0, sizeof(st));
    st.mains = MAINS_STATE_PRESENT;
    st.load = LOAD_STATE_ON;
    telemetry_snapshot_build(&snap, &st, 5400);
    ASSERT(snap.battery_pct == 0);
    n = telemetry_encode(&snap, frame, sizeof(frame));
    assert_gateway_decodes(frame, n,
                           4, TELEMETRY_FLAG_AC_PRESENT | TELEMETRY_FLAG_LOAD_ON, 0);

    /* Invalid args. */
    ASSERT(telemetry_encode(&snap, frame, 2) == 0);
    ASSERT(telemetry_encode(NULL, frame, sizeof(frame)) == 0);

    /* Change detection. */
    telemetry_trigger_t a = telemetry_trigger_from_state(&st);
    telemetry_trigger_t b = a;
    ASSERT(telemetry_trigger_changed(&a, &b) == false);
    b.load = LOAD_STATE_OFF;
    ASSERT(telemetry_trigger_changed(&a, &b) == true);

    printf("test_telemetry_codec: OK\n");
    return 0;
}
