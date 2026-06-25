#include "board_gpio.h"

#include "platform_init.h"

#ifdef DP_LUMEN_PLATFORM_STUB
static bool s_lamp;
static bool s_led_status;
static bool s_led_battery;
static bool s_led_load;
static bool s_btn_power;
static bool s_btn_test;
#endif

void board_gpio_init(void)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    s_lamp = false;
    s_led_status = false;
    s_led_battery = false;
    s_led_load = false;
    s_btn_power = false;
    s_btn_test = false;
#else
    platform_gpio_init();
#endif
}

bool board_gpio_read_btn_power(void)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    return s_btn_power;
#else
    return platform_gpio_read(BOARD_GPIO_BTN_POWER);
#endif
}

bool board_gpio_read_btn_test(void)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    return s_btn_test;
#else
    return platform_gpio_read(BOARD_GPIO_BTN_TEST);
#endif
}

void board_gpio_set_lamp(bool on)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    s_lamp = on;
#else
    platform_gpio_write(BOARD_GPIO_LAMP_CTRL, on);
#endif
}

void board_gpio_set_led_status(bool on)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    s_led_status = on;
#else
    platform_gpio_write(BOARD_GPIO_LED_STATUS, on);
#endif
}

void board_gpio_set_led_battery(bool on)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    s_led_battery = on;
#else
    platform_gpio_write(BOARD_GPIO_LED_BATTERY, on);
#endif
}

void board_gpio_set_led_load(bool on)
{
#ifdef DP_LUMEN_PLATFORM_STUB
    s_led_load = on;
#else
    platform_gpio_write(BOARD_GPIO_LED_LOAD, on);
#endif
}
