#include "board_adc.h"
#include "board_gpio.h"
#include "debounce.h"
#include "lora_radio.h"
#include "luminaire_sm.h"
#include "platform_init.h"
#include "platform_sleep.h"
#include "platform_tick.h"
#include "telemetry.h"
#include "telemetry_codec.h"
#include "uart_diag.h"

/* 15 min heartbeat at the 50 ms tick (FR-019). */
#define TELEMETRY_HEARTBEAT_TICKS (15u * 60u * 1000u / 50u)

static luminaire_state_t s_state;
static debounce_t s_db_power;
static debounce_t s_db_test;

static telemetry_trigger_t s_last_trigger;
static uint32_t s_ticks_since_tx;

static void telemetry_service(const luminaire_state_t *st, uint32_t vbat_mv, bool force)
{
    /* No transmission while in undervoltage protection (radio idle). */
    if (st->mode == OPERATING_MODE_DEEP_SLEEP) {
        return;
    }

    telemetry_trigger_t current = telemetry_trigger_from_state(st);
    bool changed = telemetry_trigger_changed(&current, &s_last_trigger);

    if (!force && !changed && s_ticks_since_tx < TELEMETRY_HEARTBEAT_TICKS) {
        return;
    }

    telemetry_snapshot_t snap;
    telemetry_snapshot_build(&snap, st, vbat_mv);

    uint8_t frame[TELEMETRY_FRAME_LEN];
    size_t len = telemetry_encode(&snap, frame, sizeof(frame));

    if (len > 0 && lora_radio_send(frame, len)) {
        s_last_trigger = current;
        s_ticks_since_tx = 0;
    }
    /* On busy/failure keep s_last_trigger unchanged so the next tick retries. */
}

int main(void)
{
    platform_init();
    platform_tick_init();
    platform_sleep_init();
    board_gpio_init();
    board_adc_init();
    uart_diag_init();
    lora_radio_init();
    debounce_init(&s_db_power);
    debounce_init(&s_db_test);

    bool mains = board_adc_mains_present();
    uint32_t vbat = board_adc_read_vbat_mv();
    luminaire_sm_init(&s_state, mains, vbat);
    luminaire_sm_apply_outputs(&s_state);
    uart_diag_emit(&s_state, BTN_DIAG_NONE);

    s_last_trigger = telemetry_trigger_from_state(&s_state);
    s_ticks_since_tx = 0;
    telemetry_service(&s_state, vbat, true);

    while (1) {
        if (platform_sleep_active() || s_state.sleep_requested) {
            platform_sleep_enter();
            platform_sleep_poll_wake();
            if (platform_sleep_active()) {
                continue;
            }
            s_state.sleep_requested = false;
            mains = board_adc_mains_present();
            vbat = board_adc_read_vbat_mv();
            luminaire_sm_init(&s_state, mains, vbat);
        }

        platform_tick_wait();

        vbat = board_adc_read_vbat_mv();
        mains = board_adc_mains_present();

        bool btn_power = debounce_tick(&s_db_power, board_gpio_read_btn_power());
        bool btn_test = debounce_tick(&s_db_test, board_gpio_read_btn_test());

        luminaire_inputs_t inputs = {
            .mains_present = mains,
            .vbat_mv = vbat,
            .btn_power_press = btn_power,
            .btn_test_press = btn_test,
        };

        btn_diag_t btn_diag = BTN_DIAG_NONE;
        luminaire_sm_tick(&s_state, &inputs, &btn_diag);
        luminaire_sm_apply_outputs(&s_state);
        uart_diag_emit(&s_state, btn_diag);

        if (s_ticks_since_tx < TELEMETRY_HEARTBEAT_TICKS) {
            s_ticks_since_tx++;
        }
        lora_radio_process();
        telemetry_service(&s_state, vbat, false);

        if (s_state.sleep_requested) {
            platform_sleep_enter();
        }
    }

    return 0;
}
