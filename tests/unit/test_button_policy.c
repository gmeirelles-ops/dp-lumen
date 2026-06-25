#include <stdio.h>
#include <stdlib.h>

#include "button_policy.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while (0)

int main(void)
{
    ASSERT(button_policy_eval(BUTTON_SOURCE_TEST, true, false) == BUTTON_ACTION_ACCEPT);
    ASSERT(button_policy_eval(BUTTON_SOURCE_TEST, false, false) == BUTTON_ACTION_IGNORE);
    ASSERT(button_policy_eval(BUTTON_SOURCE_POWER, false, false) == BUTTON_ACTION_ACCEPT);
    ASSERT(button_policy_eval(BUTTON_SOURCE_POWER, true, false) == BUTTON_ACTION_IGNORE);
    ASSERT(button_policy_eval(BUTTON_SOURCE_POWER, false, true) == BUTTON_ACTION_IGNORE);
    ASSERT(button_policy_eval(BUTTON_SOURCE_TEST, true, true) == BUTTON_ACTION_IGNORE);

    printf("test_button_policy: OK\n");
    return 0;
}
