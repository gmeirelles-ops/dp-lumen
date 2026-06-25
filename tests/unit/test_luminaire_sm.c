#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luminaire_sm.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while (0)

static void tick(luminaire_state_t *st, luminaire_inputs_t in)
{
    btn_diag_t btn = BTN_DIAG_NONE;
    luminaire_sm_tick(st, &in, &btn);
}

/* QS-03: grid loss -> lamps auto on */
static void test_qs03_grid_loss_auto_on(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, true, 7000);

    luminaire_inputs_t in = { false, 6800, false, false };
    for (int i = 0; i < 4; i++) {
        tick(&st, in);
    }

    ASSERT(st.mode == OPERATING_MODE_BATTERY_EMERGENCY);
    ASSERT(st.load == LOAD_STATE_ON);
    ASSERT(st.led_status == false);
    ASSERT(st.led_load == false);
}

/* QS-04: btn.power toggles load in battery mode */
static void test_qs04_power_toggle_battery(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, false, 6200);

    luminaire_inputs_t in = { false, 6200, true, false };
    tick(&st, in);
    ASSERT(st.load == LOAD_STATE_OFF);

    in.btn_power_press = true;
    tick(&st, in);
    ASSERT(st.load == LOAD_STATE_ON);
}

/* QS-05: low battery indicator */
static void test_qs05_low_battery_led(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, false, 5750);

    luminaire_inputs_t in = { false, 5750, false, false };
    tick(&st, in);

    ASSERT(st.battery == BATTERY_BAND_LOW);
    ASSERT(st.led_battery == true);
}

/* QS-07: deep sleep at critical, wake on VBAT > 5.6V */
static void test_qs07_deep_sleep_wake(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, false, 5350);

    luminaire_inputs_t in = { false, 5350, false, false };
    tick(&st, in);
    ASSERT(st.mode == OPERATING_MODE_DEEP_SLEEP);
    ASSERT(st.sleep_requested == true);

    in.vbat_mv = 5650;
    tick(&st, in);
    ASSERT(st.mode == OPERATING_MODE_BATTERY_EMERGENCY);
    ASSERT(st.load == LOAD_STATE_ON);
}

/* QS-01: mains idle lamps off, led.status on */
static void test_qs01_mains_idle(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, true, 7000);

    ASSERT(st.mode == OPERATING_MODE_MAINS_PRESENT);
    ASSERT(st.load == LOAD_STATE_OFF);
    ASSERT(st.led_status == true);
    ASSERT(st.led_load == true);
}

/* QS-02: btn.test toggles lamps on mains */
static void test_qs02_test_toggle_mains(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, true, 7000);

    luminaire_inputs_t in = { true, 7000, false, true };
    tick(&st, in);
    ASSERT(st.load == LOAD_STATE_ON);
    ASSERT(st.test_session.active == true);

    in.btn_test_press = true;
    tick(&st, in);
    ASSERT(st.load == LOAD_STATE_OFF);
}

/* QS-06: test session holds lamps on when mains returns */
static void test_qs06_test_session_mains_return(void)
{
    luminaire_state_t st;
    luminaire_sm_init(&st, true, 7000);

    luminaire_inputs_t in = { true, 7000, false, true };
    tick(&st, in);
    ASSERT(st.load == LOAD_STATE_ON);

    in.btn_test_press = false;
    in.mains_present = false;
    for (int i = 0; i < 4; i++) {
        tick(&st, in);
    }
    ASSERT(st.mode == OPERATING_MODE_BATTERY_EMERGENCY);

    in.mains_present = true;
    for (int i = 0; i < 4; i++) {
        tick(&st, in);
    }
    ASSERT(st.mode == OPERATING_MODE_MAINS_PRESENT);
    ASSERT(st.load == LOAD_STATE_ON);
}

int main(void)
{
    test_qs01_mains_idle();
    test_qs02_test_toggle_mains();
    test_qs03_grid_loss_auto_on();
    test_qs04_power_toggle_battery();
    test_qs05_low_battery_led();
    test_qs06_test_session_mains_return();
    test_qs07_deep_sleep_wake();

    printf("test_luminaire_sm: OK\n");
    return 0;
}
