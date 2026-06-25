#ifndef UART_DIAG_H
#define UART_DIAG_H

#include "luminaire_sm.h"

void uart_diag_init(void);

void uart_diag_emit(const luminaire_state_t *st, btn_diag_t btn);

#endif
