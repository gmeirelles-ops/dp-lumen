#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

#define DEBOUNCE_MS 50
#define DEBOUNCE_TICKS 1

typedef struct {
    bool stable_pressed;
    bool last_raw_pressed;
    uint8_t stable_ticks;
} debounce_t;

void debounce_init(debounce_t *db);

/* Returns true once per debounced press (active-low button). */
bool debounce_tick(debounce_t *db, bool raw_pressed);

#endif
