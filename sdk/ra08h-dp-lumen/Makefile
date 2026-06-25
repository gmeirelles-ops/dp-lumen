# Tremo/ASR6601 SDK location. Default matches the devcontainer (image ra08h-env),
# where the SDK is mounted at /sdk. Override via env for other setups.
TREMO_SDK_PATH ?= /sdk

PROJECT := $(notdir $(CURDIR))

DP_LUMEN_ROOT := ../..

DP_LUMEN_SRC := \
    $(DP_LUMEN_ROOT)/main/main.c \
    $(DP_LUMEN_ROOT)/components/domain/battery_band.c \
    $(DP_LUMEN_ROOT)/components/domain/button_policy.c \
    $(DP_LUMEN_ROOT)/components/domain/luminaire_sm.c \
    $(DP_LUMEN_ROOT)/components/domain/telemetry.c \
    $(DP_LUMEN_ROOT)/components/protocol/telemetry_codec.c \
    $(DP_LUMEN_ROOT)/components/connectivity/lora_radio.c \
    $(DP_LUMEN_ROOT)/components/drivers/board_gpio.c \
    $(DP_LUMEN_ROOT)/components/drivers/board_adc.c \
    $(DP_LUMEN_ROOT)/components/drivers/debounce.c \
    $(DP_LUMEN_ROOT)/components/platform/ra08h/platform_init.c \
    $(DP_LUMEN_ROOT)/components/platform/ra08h/platform_tick.c \
    $(DP_LUMEN_ROOT)/components/platform/ra08h/platform_sleep.c \
    $(DP_LUMEN_ROOT)/components/platform/ra08h/platform_uart.c \
    $(DP_LUMEN_ROOT)/components/diag/uart_diag.c

$(PROJECT)_SOURCE := $(wildcard src/*.c) \
    $(DP_LUMEN_SRC) \
    $(TREMO_SDK_PATH)/platform/system/printf-stdarg.c \
    $(TREMO_SDK_PATH)/platform/system/system_cm4.c \
    $(TREMO_SDK_PATH)/platform/system/startup_cm4.S \
    $(wildcard $(TREMO_SDK_PATH)/drivers/peripheral/src/*.c) \
    $(wildcard $(TREMO_SDK_PATH)/lora/system/*.c) \
    $(wildcard $(TREMO_SDK_PATH)/lora/system/crypto/*.c) \
    $(wildcard $(TREMO_SDK_PATH)/lora/radio/sx126x/*.c) \
    $(wildcard $(TREMO_SDK_PATH)/lora/driver/*.c)

$(PROJECT)_INC_PATH := inc \
    $(DP_LUMEN_ROOT)/main \
    $(DP_LUMEN_ROOT)/components/domain \
    $(DP_LUMEN_ROOT)/components/protocol \
    $(DP_LUMEN_ROOT)/components/connectivity \
    $(DP_LUMEN_ROOT)/components/drivers \
    $(DP_LUMEN_ROOT)/components/platform/ra08h \
    $(DP_LUMEN_ROOT)/components/diag \
    $(TREMO_SDK_PATH)/platform/CMSIS \
    $(TREMO_SDK_PATH)/platform/common \
    $(TREMO_SDK_PATH)/platform/system \
    $(TREMO_SDK_PATH)/drivers/crypto/inc \
    $(TREMO_SDK_PATH)/drivers/peripheral/inc \
    $(TREMO_SDK_PATH)/lora/driver/ \
    $(TREMO_SDK_PATH)/lora/system/ \
    $(TREMO_SDK_PATH)/lora/radio/ \
    $(TREMO_SDK_PATH)/lora/radio/sx126x/

$(PROJECT)_CFLAGS  := -Wall -Os -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -std=gnu99 -fno-builtin-printf -fno-builtin-sprintf -fno-builtin-snprintf
$(PROJECT)_DEFINES := -DCONFIG_DEBUG_UART=UART0 -DUSE_MODEM_LORA -DREGION_CN470

$(PROJECT)_LDFLAGS := -Wl,--gc-sections -Wl,--wrap=printf -Wl,--wrap=sprintf -Wl,--wrap=snprintf

$(PROJECT)_LIBS := $(TREMO_SDK_PATH)/drivers/crypto/lib/libcrypto.a

$(PROJECT)_LINK_LD := cfg/gcc.ld

SERIAL_PORT        ?= /dev/ttyACM0
SERIAL_BAUDRATE    ?= 921600
$(PROJECT)_ADDRESS := 0x08000000

##################################################################################################
include $(TREMO_SDK_PATH)/build/make/common.mk
