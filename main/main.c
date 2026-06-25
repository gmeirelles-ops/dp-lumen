#include "board_adc.h"
#include "board_gpio.h"
#include "debounce.h"
#include "luminaire_sm.h"
#include "platform_init.h"
#include "platform_sleep.h"
#include "platform_tick.h"
#include "uart_diag.h"

static luminaire_state_t s_state;
static debounce_t s_db_power;
static debounce_t s_db_test;

int main(void)
{
    platform_init();
    platform_tick_init();
    platform_sleep_init();
    board_gpio_init();
    board_adc_init();
    uart_diag_init();
    debounce_init(&s_db_power);
    debounce_init(&s_db_test);

    bool mains = board_adc_mains_present();
    uint32_t vbat = board_adc_read_vbat_mv();
    luminaire_sm_init(&s_state, mains, vbat);
    luminaire_sm_apply_outputs(&s_state);
    uart_diag_emit(&s_state, BTN_DIAG_NONE);

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

        if (s_state.sleep_requested) {
            platform_sleep_enter();
        }
    }

    return 0;
}
