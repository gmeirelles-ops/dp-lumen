#ifndef BOARD_ADC_H
#define BOARD_ADC_H

#include <stdbool.h>
#include <stdint.h>

#define BOARD_ADC_VREF_MV           3300
#define BOARD_ADC_DIVIDER_RATIO     11
#define BOARD_ADC_SAMPLES           8

#define BOARD_ADC_MAINS_ENTER_MV    11000
#define BOARD_ADC_MAINS_EXIT_MV     9900
#define BOARD_ADC_MAINS_STABLE_MS   200
#define BOARD_ADC_TICK_MS           50
#define BOARD_ADC_VBAT_WAKE_MV      5600

void board_adc_init(void);

uint32_t board_adc_read_vbat_mv(void);

bool board_adc_mains_present(void);

#ifdef DP_LUMEN_PLATFORM_STUB
void board_adc_stub_set_vbat_mv(uint32_t mv);
void board_adc_stub_set_mains_mv(uint32_t mv);
#endif

#endif
