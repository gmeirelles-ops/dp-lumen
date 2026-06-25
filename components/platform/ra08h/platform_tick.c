#include "platform_tick.h"

#ifndef DP_LUMEN_PLATFORM_STUB
#include "tremo_delay.h"
#else
#include <unistd.h>
#endif

static uint32_t s_millis;

void platform_tick_init(void)
{
    s_millis = 0;
}

void platform_tick_wait(void)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    usleep(50000);
#else
    delay_ms(50);
#endif
    s_millis += 50;
}

uint32_t platform_millis(void)
{
    return s_millis;
}
