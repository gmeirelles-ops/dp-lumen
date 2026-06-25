#include "debounce.h"

#include <stddef.h>

void debounce_init(debounce_t *db)
{
    if (db == NULL) {
        return;
    }

    db->stable_pressed = false;
    db->last_raw_pressed = false;
    db->stable_ticks = 0;
}

bool debounce_tick(debounce_t *db, bool raw_pressed)
{
    bool press_edge = false;

    if (db == NULL) {
        return false;
    }

    if (raw_pressed == db->stable_pressed) {
        db->stable_ticks = DEBOUNCE_TICKS;
    } else {
        if (raw_pressed == db->last_raw_pressed) {
            if (db->stable_ticks < DEBOUNCE_TICKS) {
                db->stable_ticks++;
            }
        } else {
            db->stable_ticks = 1;
        }

        if (db->stable_ticks >= DEBOUNCE_TICKS) {
            if (raw_pressed && !db->stable_pressed) {
                press_edge = true;
            }
            db->stable_pressed = raw_pressed;
        }
    }

    db->last_raw_pressed = raw_pressed;
    return press_edge;
}
