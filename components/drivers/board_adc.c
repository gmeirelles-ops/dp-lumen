#include "board_adc.h"

#include "board_pins.h"
#include "platform_init.h"

#define MAINS_STABLE_TICKS (BOARD_ADC_MAINS_STABLE_MS / BOARD_ADC_TICK_MS)

typedef struct {
    bool present;
    uint8_t stable_count;
    bool candidate_present;
} mains_filter_t;

static mains_filter_t s_mains_filter;

#ifdef DP_LUMEN_PLATFORM_STUB
static uint32_t s_stub_vbat_mv = 7000;
static uint32_t s_stub_mains_mv = 12000;
#endif

static uint32_t counts_to_input_mv(uint16_t counts)
{
    uint32_t adc_mv = ((uint32_t)counts * BOARD_ADC_VREF_MV) / 4095;
    return adc_mv * BOARD_ADC_DIVIDER_RATIO;
}

static uint16_t read_channel_avg(board_gpio_pin_t pin)
{
    uint32_t sum = 0;

    for (int i = 0; i < BOARD_ADC_SAMPLES; i++) {
#ifdef DP_LUMEN_PLATFORM_STUB
        (void)pin;
        if (pin == BOARD_GPIO_ADC_VBAT) {
            sum += (s_stub_vbat_mv / BOARD_ADC_DIVIDER_RATIO) * 4095 / BOARD_ADC_VREF_MV;
        } else {
            sum += (s_stub_mains_mv / BOARD_ADC_DIVIDER_RATIO) * 4095 / BOARD_ADC_VREF_MV;
        }
#else
        sum += platform_adc_read(pin);
#endif
    }

    return (uint16_t)(sum / BOARD_ADC_SAMPLES);
}

static void update_mains_filter(uint32_t mains_mv)
{
    bool candidate;

    if (s_mains_filter.present) {
        candidate = mains_mv > BOARD_ADC_MAINS_EXIT_MV;
    } else {
        candidate = mains_mv >= BOARD_ADC_MAINS_ENTER_MV;
    }

    if (candidate == s_mains_filter.candidate_present) {
        if (s_mains_filter.stable_count < MAINS_STABLE_TICKS) {
            s_mains_filter.stable_count++;
        }
    } else {
        s_mains_filter.candidate_present = candidate;
        s_mains_filter.stable_count = 1;
    }

    if (s_mains_filter.stable_count >= MAINS_STABLE_TICKS) {
        s_mains_filter.present = candidate;
    }
}

void board_adc_init(void)
{
    s_mains_filter.present = false;
    s_mains_filter.stable_count = 0;
    s_mains_filter.candidate_present = false;

#ifndef DP_LUMEN_PLATFORM_STUB
    platform_adc_init();
#endif
}

uint32_t board_adc_read_vbat_mv(void)
{
    uint16_t counts = read_channel_avg(BOARD_GPIO_ADC_VBAT);
    return counts_to_input_mv(counts);
}

bool board_adc_mains_present(void)
{
    uint16_t counts = read_channel_avg(BOARD_GPIO_ADC_MAINS);
    uint32_t mains_mv = counts_to_input_mv(counts);
    update_mains_filter(mains_mv);
    return s_mains_filter.present;
}

#ifdef DP_LUMEN_PLATFORM_STUB
void board_adc_stub_set_vbat_mv(uint32_t mv)
{
    s_stub_vbat_mv = mv;
}

void board_adc_stub_set_mains_mv(uint32_t mv)
{
    s_stub_mains_mv = mv;
}
#endif
