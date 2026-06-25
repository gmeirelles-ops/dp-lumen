#include "uart_diag.h"

#include <stdio.h>
#include <string.h>

#include "platform_tick.h"
#include "platform_uart.h"

#define UART_DIAG_MAX_LINES_PER_SEC 10
#define UART_DIAG_WINDOW_MS 1000

typedef struct {
    operating_mode_t mode;
    mains_state_t mains;
    battery_band_t battery;
    load_state_t load;
    bool led_status;
    bool led_battery;
    bool led_load;
    uint32_t window_start_ms;
    uint8_t lines_in_window;
} uart_diag_cache_t;

static uart_diag_cache_t s_cache;
static bool s_cache_valid;

static const char *mode_str(operating_mode_t mode)
{
    switch (mode) {
    case OPERATING_MODE_MAINS_PRESENT:
        return "mains";
    case OPERATING_MODE_BATTERY_EMERGENCY:
        return "battery";
    case OPERATING_MODE_DEEP_SLEEP:
        return "sleep";
    default:
        return "unknown";
    }
}

static const char *band_str(battery_band_t band)
{
    switch (band) {
    case BATTERY_BAND_CHARGED:
        return "charged";
    case BATTERY_BAND_NORMAL:
        return "normal";
    case BATTERY_BAND_LOW:
        return "low";
    case BATTERY_BAND_CRITICAL:
        return "critical";
    default:
        return "unknown";
    }
}

static const char *btn_str(btn_diag_t btn)
{
    switch (btn) {
    case BTN_DIAG_POWER:
        return "power";
    case BTN_DIAG_TEST:
        return "test";
    case BTN_DIAG_IGNORED_POWER:
        return "ignored:power";
    case BTN_DIAG_IGNORED_TEST:
        return "ignored:test";
    default:
        return "none";
    }
}

static bool state_changed(const luminaire_state_t *st, btn_diag_t btn)
{
    if (!s_cache_valid) {
        return true;
    }

    if (btn != BTN_DIAG_NONE) {
        return true;
    }

    return s_cache.mode != st->mode ||
           s_cache.mains != st->mains ||
           s_cache.battery != st->battery ||
           s_cache.load != st->load ||
           s_cache.led_status != st->led_status ||
           s_cache.led_battery != st->led_battery ||
           s_cache.led_load != st->led_load;
}

static bool rate_limit_ok(void)
{
    uint32_t now = platform_millis();

    if (!s_cache_valid || (now - s_cache.window_start_ms) >= UART_DIAG_WINDOW_MS) {
        s_cache.window_start_ms = now;
        s_cache.lines_in_window = 0;
    }

    if (s_cache.lines_in_window >= UART_DIAG_MAX_LINES_PER_SEC) {
        return false;
    }

    s_cache.lines_in_window++;
    return true;
}

void uart_diag_init(void)
{
    memset(&s_cache, 0, sizeof(s_cache));
    s_cache_valid = false;
}

void uart_diag_emit(const luminaire_state_t *st, btn_diag_t btn)
{
    char line[160];
    char led_triplet[4];

    if (st == NULL) {
        return;
    }

    if (!state_changed(st, btn)) {
        return;
    }

    if (!rate_limit_ok()) {
        return;
    }

    led_triplet[0] = st->led_status ? '1' : '0';
    led_triplet[1] = st->led_battery ? '1' : '0';
    led_triplet[2] = st->led_load ? '1' : '0';
    led_triplet[3] = '\0';

    snprintf(line, sizeof(line),
             "DP-LUMEN mode=%s mains=%d batt=%s load=%s led=%s btn=%s\r\n",
             mode_str(st->mode),
             st->mains == MAINS_STATE_PRESENT ? 1 : 0,
             band_str(st->battery),
             st->load == LOAD_STATE_ON ? "on" : "off",
             led_triplet,
             btn_str(btn));

    platform_uart_write(line, strlen(line));

    s_cache.mode = st->mode;
    s_cache.mains = st->mains;
    s_cache.battery = st->battery;
    s_cache.load = st->load;
    s_cache.led_status = st->led_status;
    s_cache.led_battery = st->led_battery;
    s_cache.led_load = st->led_load;
    s_cache_valid = true;
}
