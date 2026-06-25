#ifndef BATTERY_BAND_H
#define BATTERY_BAND_H

#include <stdint.h>

typedef enum {
    BATTERY_BAND_CHARGED,
    BATTERY_BAND_NORMAL,
    BATTERY_BAND_LOW,
    BATTERY_BAND_CRITICAL,
} battery_band_t;

typedef struct {
    battery_band_t band;
} battery_band_hysteresis_t;

#define BATTERY_MV_CHARGED      6700
#define BATTERY_MV_LOW_ENTER    5800
#define BATTERY_MV_LOW_LEAVE    5900
#define BATTERY_MV_CRITICAL     5400
#define BATTERY_MV_WAKE         5600

void battery_band_hysteresis_init(battery_band_hysteresis_t *hyst);

battery_band_t battery_band_from_mv(uint32_t vbat_mv, battery_band_hysteresis_t *hyst);

/* Battery charge percentage for telemetry (pure).
 * Linear approximation, clamped to [0,100]:
 *   5.4 V (critical) -> 0 %, 6.7 V (charged) -> 100 %.
 * Lead-acid SOC is non-linear; this is a fleet-monitoring approximation. */
uint8_t battery_pct_from_mv(uint32_t vbat_mv);

#endif
