#ifndef PLATFORM_SLEEP_H
#define PLATFORM_SLEEP_H

#include <stdbool.h>

void platform_sleep_init(void);
bool platform_sleep_active(void);
void platform_sleep_enter(void);
void platform_sleep_poll_wake(void);

#endif
