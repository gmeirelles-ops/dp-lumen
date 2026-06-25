#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdbool.h>
#include <stdint.h>

#include "luminaire_sm.h"

/* Provisional product type id (FR-017). Not final. */
#ifndef TELEMETRY_PRODUCT_ID
#define TELEMETRY_PRODUCT_ID 4
#endif

/* Logical telemetry payload sent over LoRa P2P (see contracts/telemetry-lora.md). */
typedef struct {
    uint8_t product_id;
    bool ac_present;
    bool load_on;
    uint8_t battery_pct;
} telemetry_snapshot_t;

/* Fields that trigger an on-change uplink: mode, battery band, load, mains. */
typedef struct {
    operating_mode_t mode;
    battery_band_t battery;
    load_state_t load;
    mains_state_t mains;
} telemetry_trigger_t;

void telemetry_snapshot_build(telemetry_snapshot_t *snap,
                              const luminaire_state_t *st,
                              uint32_t vbat_mv);

telemetry_trigger_t telemetry_trigger_from_state(const luminaire_state_t *st);

bool telemetry_trigger_changed(const telemetry_trigger_t *a,
                               const telemetry_trigger_t *b);

#endif
