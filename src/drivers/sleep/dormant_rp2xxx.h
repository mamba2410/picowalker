#ifndef PW_POWER_PICO_H
#define PW_POWER_PICO_H

#include "../../picowalker-defs.h"

#define USER_IDLE_TIMEOUT_MS (30*1000)

#define USER_IDLE_ALARM_NUM (0u)

extern pw_wake_reason_t wake_reason;
extern volatile bool power_sleep_enabled;

void user_idle_callback();
void set_user_idle_timer();

#endif /* PW_POWER_PICO_H */
