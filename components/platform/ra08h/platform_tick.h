#ifndef PLATFORM_TICK_H
#define PLATFORM_TICK_H

#include <stdint.h>

void platform_tick_init(void);
void platform_tick_wait(void);
uint32_t platform_millis(void);

#endif
