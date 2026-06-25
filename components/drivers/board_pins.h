#ifndef BOARD_PINS_H
#define BOARD_PINS_H

/* RA-08H pin map per constitution v2.0.0 */
typedef enum {
    BOARD_GPIO_ADC_VBAT   = 8,
    BOARD_GPIO_ADC_MAINS  = 11,
    BOARD_GPIO_BTN_TEST   = 9,
    BOARD_GPIO_BTN_POWER  = 15,
    BOARD_GPIO_LED_STATUS = 4,
    BOARD_GPIO_LED_BATTERY = 5,
    BOARD_GPIO_LED_LOAD   = 7,
    BOARD_GPIO_LAMP_CTRL  = 14,
} board_gpio_pin_t;

#endif
