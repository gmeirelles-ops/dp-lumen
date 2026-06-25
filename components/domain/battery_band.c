#include "battery_band.h"

#include <stddef.h>

void battery_band_hysteresis_init(battery_band_hysteresis_t *hyst)
{
    if (hyst != NULL) {
        hyst->band = BATTERY_BAND_NORMAL;
    }
}

battery_band_t battery_band_from_mv(uint32_t vbat_mv, battery_band_hysteresis_t *hyst)
{
    if (vbat_mv <= BATTERY_MV_CRITICAL) {
        if (hyst != NULL) {
            hyst->band = BATTERY_BAND_CRITICAL;
        }
        return BATTERY_BAND_CRITICAL;
    }

    if (vbat_mv >= BATTERY_MV_CHARGED) {
        if (hyst != NULL) {
            hyst->band = BATTERY_BAND_CHARGED;
        }
        return BATTERY_BAND_CHARGED;
    }

    if (hyst != NULL && hyst->band == BATTERY_BAND_LOW) {
        if (vbat_mv > BATTERY_MV_LOW_LEAVE) {
            hyst->band = BATTERY_BAND_NORMAL;
            return BATTERY_BAND_NORMAL;
        }
        hyst->band = BATTERY_BAND_LOW;
        return BATTERY_BAND_LOW;
    }

    if (vbat_mv <= BATTERY_MV_LOW_ENTER) {
        if (hyst != NULL) {
            hyst->band = BATTERY_BAND_LOW;
        }
        return BATTERY_BAND_LOW;
    }

    if (hyst != NULL) {
        hyst->band = BATTERY_BAND_NORMAL;
    }
    return BATTERY_BAND_NORMAL;
}

uint8_t battery_pct_from_mv(uint32_t vbat_mv)
{
    if (vbat_mv <= BATTERY_MV_CRITICAL) {
        return 0;
    }
    if (vbat_mv >= BATTERY_MV_CHARGED) {
        return 100;
    }

    uint32_t span = BATTERY_MV_CHARGED - BATTERY_MV_CRITICAL;
    uint32_t pct = ((vbat_mv - BATTERY_MV_CRITICAL) * 100u) / span;
    return (uint8_t)pct;
}
