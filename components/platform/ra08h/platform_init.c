#include "platform_init.h"

#ifndef DP_LUMEN_PLATFORM_STUB

#include <stdbool.h>

#include "board_pins.h"
#include "tremo_adc.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"

#include "platform_uart.h"

#define DP_GPIO_PORT GPIOA

static adc_sample_chan_t pin_to_adc_chan(board_gpio_pin_t pin)
{
    if (pin == BOARD_GPIO_ADC_MAINS) {
        return ADC_SAMPLE_CHAN_2;
    }
    return ADC_SAMPLE_CHAN_1;
}

void platform_init(void)
{
    rcc_set_adc_clk_source(RCC_ADC_CLK_SOURCE_RCO48M);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);

    platform_uart_init();
    platform_gpio_init();
    platform_adc_init();
}

void platform_gpio_init(void)
{
    gpio_init(DP_GPIO_PORT, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(DP_GPIO_PORT, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(DP_GPIO_PORT, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(DP_GPIO_PORT, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(DP_GPIO_PORT, GPIO_PIN_9, GPIO_MODE_INPUT_PULL_UP);
    gpio_init(DP_GPIO_PORT, GPIO_PIN_15, GPIO_MODE_INPUT_PULL_UP);

    gpio_init(DP_GPIO_PORT, GPIO_PIN_8, GPIO_MODE_ANALOG);
    gpio_init(DP_GPIO_PORT, GPIO_PIN_11, GPIO_MODE_ANALOG);
}

bool platform_gpio_read(board_gpio_pin_t pin)
{
    gpio_level_t level = gpio_read(DP_GPIO_PORT, (uint8_t)pin);
    return level == GPIO_LEVEL_LOW;
}

void platform_gpio_write(board_gpio_pin_t pin, bool high)
{
    gpio_write(DP_GPIO_PORT, (uint8_t)pin,
               high ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

void platform_adc_init(void)
{
    adc_init();
    adc_config_clock_division(20);
    adc_config_conv_mode(ADC_CONV_MODE_SINGLE);
    adc_enable(true);
}

uint16_t platform_adc_read(board_gpio_pin_t pin)
{
    adc_sample_chan_t chan = pin_to_adc_chan(pin);

    adc_config_sample_sequence(ADC_SAMPLE_SEQ_CHAN_0, chan);
    adc_start(true);

    while (!adc_get_interrupt_status(ADC_ISR_EOC)) {
    }

    uint16_t value = adc_get_data();
    adc_start(false);
    return value;
}

#else

#include <stdbool.h>
#include <stdio.h>

#include "platform_uart.h"

void platform_init(void)
{
    platform_uart_init();
    platform_gpio_init();
}

void platform_gpio_init(void)
{
}

bool platform_gpio_read(board_gpio_pin_t pin)
{
    (void)pin;
    return false;
}

void platform_gpio_write(board_gpio_pin_t pin, bool high)
{
    (void)pin;
    (void)high;
}

void platform_adc_init(void)
{
}

uint16_t platform_adc_read(board_gpio_pin_t pin)
{
    (void)pin;
    return 0;
}

#endif
