#include "platform_uart.h"

#include <stddef.h>

#ifndef DP_LUMEN_PLATFORM_STUB
#include "tremo_gpio.h"
#include "tremo_uart.h"
#else
#include <stdio.h>
#endif

#ifndef CONFIG_DEBUG_UART
#define CONFIG_DEBUG_UART UART0
#endif

void platform_uart_init(void)
{
#ifndef DP_LUMEN_PLATFORM_STUB
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    uart_config_t uart_config;
    uart_config_init(&uart_config);
    uart_config.baudrate = UART_BAUDRATE_115200;
    uart_init(CONFIG_DEBUG_UART, &uart_config);
    uart_cmd(CONFIG_DEBUG_UART, ENABLE);
#endif
}

void platform_uart_write(const char *data, size_t len)
{
    if (data == NULL || len == 0) {
        return;
    }

#ifdef DP_LUMEN_PLATFORM_STUB
    fwrite(data, 1, len, stdout);
    fflush(stdout);
#else
    for (size_t i = 0; i < len; i++) {
        uart_send_data(CONFIG_DEBUG_UART, (uint8_t)data[i]);
    }
#endif
}
