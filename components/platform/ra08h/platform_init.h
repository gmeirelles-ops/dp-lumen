#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

#include <stdbool.h>
#include <stdint.h>

#include "board_pins.h"

void platform_init(void);
void platform_gpio_init(void);
bool platform_gpio_read(board_gpio_pin_t pin);
void platform_gpio_write(board_gpio_pin_t pin, bool high);
void platform_adc_init(void);
uint16_t platform_adc_read(board_gpio_pin_t pin);

#endif
