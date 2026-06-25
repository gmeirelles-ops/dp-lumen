#ifndef LORA_CONFIG_H
#define LORA_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tremo_gpio.h"

/* RA-08H module-internal antenna-switch controls (mirrored from the
 * ra08h-dp-comm reference). Used by the SX126x board driver. These are not
 * application I/O and do not collide with the constitution GPIO table.
 * Confirm against the dp-lumen schematic before flashing. */
#define CONFIG_LORA_RFSW_CTRL_GPIOX GPIOD
#define CONFIG_LORA_RFSW_CTRL_PIN   GPIO_PIN_11

#define CONFIG_LORA_RFSW_VDD_GPIOX GPIOA
#define CONFIG_LORA_RFSW_VDD_PIN   GPIO_PIN_10

#ifdef __cplusplus
}
#endif

#endif /* LORA_CONFIG_H */
