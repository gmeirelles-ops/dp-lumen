#ifndef BOARD_GPIO_H
#define BOARD_GPIO_H

#include <stdbool.h>

#include "board_pins.h"

void board_gpio_init(void);

bool board_gpio_read_btn_power(void);
bool board_gpio_read_btn_test(void);

void board_gpio_set_lamp(bool on);
void board_gpio_set_led_status(bool on);
void board_gpio_set_led_battery(bool on);
void board_gpio_set_led_load(bool on);

#endif
