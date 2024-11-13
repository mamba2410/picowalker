#ifndef PW_POWER_PICO_H
#define PW_POWER_PICO_H

#define USER_IDLE_TIMEOUT_MS (10*1000)

#define USER_IDLE_ALARM_NUM (0u)

void user_idle_callback();
void set_user_idle_timer();

#endif /* PW_POWER_PICO_H */

