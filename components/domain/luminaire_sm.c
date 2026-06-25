#include "luminaire_sm.h"

#include <stddef.h>

#include "board_gpio.h"
#include "button_policy.h"

static battery_band_hysteresis_t s_battery_hyst;

static void update_indicators(luminaire_state_t *st)
{
    st->led_status = (st->mains == MAINS_STATE_PRESENT);
    st->led_battery = (st->battery == BATTERY_BAND_LOW ||
                       st->battery == BATTERY_BAND_CRITICAL);
    st->led_load = (st->mains == MAINS_STATE_PRESENT &&
                    st->battery == BATTERY_BAND_CHARGED);

    if (st->mode == OPERATING_MODE_DEEP_SLEEP) {
        st->led_status = false;
        st->led_battery = false;
        st->led_load = false;
    }
}

static void enter_battery_emergency(luminaire_state_t *st)
{
    st->mode = OPERATING_MODE_BATTERY_EMERGENCY;
    st->load = LOAD_STATE_ON;
    st->sleep_requested = false;
    update_indicators(st);
}

static void enter_mains_present(luminaire_state_t *st)
{
    st->mode = OPERATING_MODE_MAINS_PRESENT;
    st->sleep_requested = false;

    if (st->test_session.active && st->test_session.load_override == LOAD_STATE_ON) {
        st->load = LOAD_STATE_ON;
    } else {
        st->load = LOAD_STATE_OFF;
        st->test_session.active = false;
    }

    update_indicators(st);
}

static void enter_deep_sleep(luminaire_state_t *st)
{
    st->mode = OPERATING_MODE_DEEP_SLEEP;
    st->load = LOAD_STATE_OFF;
    st->sleep_requested = true;
    update_indicators(st);
}

void luminaire_sm_init(luminaire_state_t *st, bool mains_present, uint32_t vbat_mv)
{
    battery_band_hysteresis_init(&s_battery_hyst);

    st->mains = mains_present ? MAINS_STATE_PRESENT : MAINS_STATE_ABSENT;
    st->battery = battery_band_from_mv(vbat_mv, &s_battery_hyst);
    st->test_session.active = false;
    st->test_session.load_override = LOAD_STATE_OFF;
    st->sleep_requested = false;

    if (mains_present) {
        st->mode = OPERATING_MODE_MAINS_PRESENT;
        st->load = LOAD_STATE_OFF;
    } else if (st->battery == BATTERY_BAND_CRITICAL) {
        st->mode = OPERATING_MODE_DEEP_SLEEP;
        st->load = LOAD_STATE_OFF;
        st->sleep_requested = true;
    } else {
        st->mode = OPERATING_MODE_BATTERY_EMERGENCY;
        st->load = LOAD_STATE_ON;
    }

    update_indicators(st);
}

void luminaire_sm_tick(luminaire_state_t *st, const luminaire_inputs_t *in, btn_diag_t *btn_out)
{
    if (st == NULL || in == NULL) {
        return;
    }

    if (btn_out != NULL) {
        *btn_out = BTN_DIAG_NONE;
    }

    st->mains = in->mains_present ? MAINS_STATE_PRESENT : MAINS_STATE_ABSENT;
    st->battery = battery_band_from_mv(in->vbat_mv, &s_battery_hyst);

    if (st->mode == OPERATING_MODE_DEEP_SLEEP) {
        bool wake_mains = in->mains_present;
        bool wake_vbat = in->vbat_mv > BATTERY_MV_WAKE;

        if (wake_mains) {
            enter_mains_present(st);
        } else if (wake_vbat) {
            enter_battery_emergency(st);
        } else {
            st->load = LOAD_STATE_OFF;
            st->sleep_requested = true;
            update_indicators(st);
            return;
        }
    }

    if (st->mode == OPERATING_MODE_MAINS_PRESENT && !in->mains_present) {
        enter_battery_emergency(st);
    } else if (st->mode == OPERATING_MODE_BATTERY_EMERGENCY && in->mains_present) {
        enter_mains_present(st);
    }

    if (st->mode == OPERATING_MODE_BATTERY_EMERGENCY &&
        st->battery == BATTERY_BAND_CRITICAL) {
        enter_deep_sleep(st);
        return;
    }

    bool deep_sleep = (st->mode == OPERATING_MODE_DEEP_SLEEP);

    if (in->btn_test_press) {
        button_action_t action = button_policy_eval(BUTTON_SOURCE_TEST, in->mains_present, deep_sleep);
        if (action == BUTTON_ACTION_ACCEPT) {
            st->load = (st->load == LOAD_STATE_ON) ? LOAD_STATE_OFF : LOAD_STATE_ON;
            st->test_session.active = true;
            st->test_session.load_override = st->load;
            if (btn_out != NULL) {
                *btn_out = BTN_DIAG_TEST;
            }
        } else if (btn_out != NULL) {
            *btn_out = BTN_DIAG_IGNORED_TEST;
        }
    }

    if (in->btn_power_press) {
        button_action_t action = button_policy_eval(BUTTON_SOURCE_POWER, in->mains_present, deep_sleep);
        if (action == BUTTON_ACTION_ACCEPT) {
            st->load = (st->load == LOAD_STATE_ON) ? LOAD_STATE_OFF : LOAD_STATE_ON;
            if (btn_out != NULL) {
                *btn_out = BTN_DIAG_POWER;
            }
        } else if (btn_out != NULL) {
            *btn_out = BTN_DIAG_IGNORED_POWER;
        }
    }

    update_indicators(st);
}

void luminaire_sm_apply_outputs(const luminaire_state_t *st)
{
    if (st == NULL) {
        return;
    }

    board_gpio_set_lamp(st->load == LOAD_STATE_ON);
    board_gpio_set_led_status(st->led_status);
    board_gpio_set_led_battery(st->led_battery);
    board_gpio_set_led_load(st->led_load);
}
