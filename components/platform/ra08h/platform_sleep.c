#include "platform_sleep.h"

#include "board_adc.h"
#include "platform_tick.h"

#ifndef DP_LUMEN_PLATFORM_STUB
#include "tremo_delay.h"
#include "tremo_pwr.h"
#include "tremo_rcc.h"
#include "platform_init.h"
#endif

#define SLEEP_POLL_MS 2000

static bool s_sleeping;
static uint32_t s_last_poll_ms;

void platform_sleep_init(void)
{
    s_sleeping = false;
    s_last_poll_ms = 0;
}

bool platform_sleep_active(void)
{
    return s_sleeping;
}

void platform_sleep_enter(void)
{
    s_sleeping = true;
    s_last_poll_ms = platform_millis();

#ifndef DP_LUMEN_PLATFORM_STUB
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
#endif
}

void platform_sleep_poll_wake(void)
{
    if (!s_sleeping) {
        return;
    }

#ifndef DP_LUMEN_PLATFORM_STUB
    platform_init();
#endif

    uint32_t now = platform_millis();
    if ((now - s_last_poll_ms) < SLEEP_POLL_MS) {
        platform_tick_wait();
        return;
    }
    s_last_poll_ms = now;

    bool mains = board_adc_mains_present();
    uint32_t vbat = board_adc_read_vbat_mv();

    if (mains || vbat > BOARD_ADC_VBAT_WAKE_MV) {
        s_sleeping = false;
        return;
    }

#ifndef DP_LUMEN_PLATFORM_STUB
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
#endif
}
