#include "telemetry.h"

#include <stddef.h>

#include "battery_band.h"

void telemetry_snapshot_build(telemetry_snapshot_t *snap,
                              const luminaire_state_t *st,
                              uint32_t vbat_mv)
{
    if (snap == NULL || st == NULL) {
        return;
    }

    snap->product_id = TELEMETRY_PRODUCT_ID;
    snap->ac_present = (st->mains == MAINS_STATE_PRESENT);
    snap->load_on = (st->load == LOAD_STATE_ON);
    snap->battery_pct = battery_pct_from_mv(vbat_mv);
}

telemetry_trigger_t telemetry_trigger_from_state(const luminaire_state_t *st)
{
    telemetry_trigger_t trig = {0};

    if (st != NULL) {
        trig.mode = st->mode;
        trig.battery = st->battery;
        trig.load = st->load;
        trig.mains = st->mains;
    }

    return trig;
}

bool telemetry_trigger_changed(const telemetry_trigger_t *a,
                               const telemetry_trigger_t *b)
{
    if (a == NULL || b == NULL) {
        return false;
    }

    return a->mode != b->mode ||
           a->battery != b->battery ||
           a->load != b->load ||
           a->mains != b->mains;
}
