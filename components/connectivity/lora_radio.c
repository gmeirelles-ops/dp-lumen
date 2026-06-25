#include "lora_radio.h"

#ifndef DP_LUMEN_PLATFORM_STUB

#include <string.h>

#include "timer.h"
#include "radio.h"
#include "rtc-board.h"

/* Radio parameters — must match the ra08h-dp-comm gateway (see contract). */
#define LORA_RF_FREQUENCY          915000000UL /* Hz */
#define LORA_TX_OUTPUT_POWER       22          /* dBm */
#define LORA_BANDWIDTH             2           /* 0:125k 1:250k 2:500k */
#define LORA_SPREADING_FACTOR      7           /* SF7 */
#define LORA_CODINGRATE            1           /* 4/5 */
#define LORA_PREAMBLE_LENGTH       8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON       false
#define LORA_TX_TIMEOUT_MS         3000

/* Telemetry frames are 4 bytes; keep a small bounded TX buffer. */
#define LORA_TX_MAX_LEN 16

static RadioEvents_t s_radio_events;
static volatile bool s_busy;
static uint8_t s_tx_buf[LORA_TX_MAX_LEN];

static void on_tx_done(void)
{
    /* Unlike the gateway bridge, do NOT reset the MCU; just free the radio. */
    s_busy = false;
}

static void on_tx_timeout(void)
{
    s_busy = false;
}

/* Uplink only: RX callbacks are required by RadioEvents_t but never armed. */
static void on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    (void)payload;
    (void)size;
    (void)rssi;
    (void)snr;
}

static void on_rx_timeout(void)
{
}

static void on_rx_error(void)
{
}

void lora_radio_init(void)
{
    s_busy = false;

    RtcInit();

    s_radio_events.TxDone = on_tx_done;
    s_radio_events.TxTimeout = on_tx_timeout;
    s_radio_events.RxDone = on_rx_done;
    s_radio_events.RxTimeout = on_rx_timeout;
    s_radio_events.RxError = on_rx_error;

    Radio.Init(&s_radio_events);
    Radio.SetChannel(LORA_RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, LORA_TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, LORA_TX_TIMEOUT_MS);
}

bool lora_radio_send(const uint8_t *frame, size_t len)
{
    if (frame == NULL || len == 0 || len > LORA_TX_MAX_LEN) {
        return false;
    }
    if (s_busy) {
        return false;
    }

    memcpy(s_tx_buf, frame, len);
    s_busy = true;
    Radio.Send(s_tx_buf, (uint8_t)len);
    return true;
}

void lora_radio_process(void)
{
    Radio.IrqProcess();
}

bool lora_radio_busy(void)
{
    return s_busy;
}

#else /* DP_LUMEN_PLATFORM_STUB — host build: no radio */

void lora_radio_init(void)
{
}

bool lora_radio_send(const uint8_t *frame, size_t len)
{
    (void)frame;
    (void)len;
    return true;
}

void lora_radio_process(void)
{
}

bool lora_radio_busy(void)
{
    return false;
}

#endif
