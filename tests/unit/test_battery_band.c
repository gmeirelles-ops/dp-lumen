#include <stdio.h>
#include <stdlib.h>

#include "battery_band.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while (0)

int main(void)
{
    battery_band_hysteresis_t hyst;
    battery_band_hysteresis_init(&hyst);

    ASSERT(battery_band_from_mv(7000, &hyst) == BATTERY_BAND_CHARGED);
    ASSERT(battery_band_from_mv(6700, &hyst) == BATTERY_BAND_CHARGED);
    ASSERT(battery_band_from_mv(6500, &hyst) == BATTERY_BAND_NORMAL);
    ASSERT(battery_band_from_mv(5900, &hyst) == BATTERY_BAND_NORMAL);

    hyst.band = BATTERY_BAND_NORMAL;
    ASSERT(battery_band_from_mv(5800, &hyst) == BATTERY_BAND_LOW);
    ASSERT(battery_band_from_mv(5850, &hyst) == BATTERY_BAND_LOW);
    ASSERT(battery_band_from_mv(5890, &hyst) == BATTERY_BAND_LOW);
    ASSERT(battery_band_from_mv(5901, &hyst) == BATTERY_BAND_NORMAL);

    ASSERT(battery_band_from_mv(5400, &hyst) == BATTERY_BAND_CRITICAL);
    ASSERT(battery_band_from_mv(5000, &hyst) == BATTERY_BAND_CRITICAL);

    /* battery_pct_from_mv: linear 5.4 V = 0 %, 6.7 V = 100 %, clamped. */
    ASSERT(battery_pct_from_mv(5000) == 0);
    ASSERT(battery_pct_from_mv(5400) == 0);
    ASSERT(battery_pct_from_mv(6050) == 50);
    ASSERT(battery_pct_from_mv(6700) == 100);
    ASSERT(battery_pct_from_mv(7000) == 100);

    printf("test_battery_band: OK\n");
    return 0;
}
