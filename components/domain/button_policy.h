#ifndef BUTTON_POLICY_H
#define BUTTON_POLICY_H

#include <stdbool.h>

typedef enum {
    BUTTON_SOURCE_POWER,
    BUTTON_SOURCE_TEST,
} button_source_t;

typedef enum {
    BUTTON_ACTION_IGNORE,
    BUTTON_ACTION_ACCEPT,
} button_action_t;

button_action_t button_policy_eval(button_source_t source, bool mains_present, bool deep_sleep);

#endif
