#ifndef PLATFORM_UART_H
#define PLATFORM_UART_H

#include <stddef.h>

void platform_uart_init(void);
void platform_uart_write(const char *data, size_t len);

#endif
