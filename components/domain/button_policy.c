#include "button_policy.h"

button_action_t button_policy_eval(button_source_t source, bool mains_present, bool deep_sleep)
{
    if (deep_sleep) {
        return BUTTON_ACTION_IGNORE;
    }

    if (source == BUTTON_SOURCE_TEST) {
        return mains_present ? BUTTON_ACTION_ACCEPT : BUTTON_ACTION_IGNORE;
    }

    if (source == BUTTON_SOURCE_POWER) {
        return mains_present ? BUTTON_ACTION_IGNORE : BUTTON_ACTION_ACCEPT;
    }

    return BUTTON_ACTION_IGNORE;
}
