#ifndef LUMINAIRE_SM_H
#define LUMINAIRE_SM_H

#include <stdbool.h>
#include <stdint.h>

#include "battery_band.h"

typedef enum {
    OPERATING_MODE_MAINS_PRESENT,
    OPERATING_MODE_BATTERY_EMERGENCY,
    OPERATING_MODE_DEEP_SLEEP,
} operating_mode_t;

typedef enum {
    LOAD_STATE_OFF,
    LOAD_STATE_ON,
} load_state_t;

typedef enum {
    MAINS_STATE_ABSENT,
    MAINS_STATE_PRESENT,
} mains_state_t;

typedef struct {
    bool active;
    load_state_t load_override;
} test_session_t;

typedef struct {
    operating_mode_t mode;
    mains_state_t mains;
    battery_band_t battery;
    load_state_t load;
    test_session_t test_session;
    bool led_status;
    bool led_battery;
    bool led_load;
    bool sleep_requested;
} luminaire_state_t;

typedef struct {
    bool mains_present;
    uint32_t vbat_mv;
    bool btn_power_press;
    bool btn_test_press;
} luminaire_inputs_t;

typedef enum {
    BTN_DIAG_NONE,
    BTN_DIAG_POWER,
    BTN_DIAG_TEST,
    BTN_DIAG_IGNORED_POWER,
    BTN_DIAG_IGNORED_TEST,
} btn_diag_t;

void luminaire_sm_init(luminaire_state_t *st, bool mains_present, uint32_t vbat_mv);

void luminaire_sm_tick(luminaire_state_t *st, const luminaire_inputs_t *in, btn_diag_t *btn_out);

void luminaire_sm_apply_outputs(const luminaire_state_t *st);

#endif
