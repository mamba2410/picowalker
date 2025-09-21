#ifndef PW_POWER_PICO_H
#define PW_POWER_PICO_H

#include "../../picowalker-defs.h"

<<<<<<< HEAD:src/drivers/power/power_pico.h
<<<<<<< HEAD
#define USER_IDLE_TIMEOUT_MS (30*1000)
=======
#define USER_IDLE_TIMEOUT_MS (10*1000)
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
#define USER_IDLE_TIMEOUT_MS (30*1000)
>>>>>>> 5cad753 (rebase survival):src/drivers/sleep/dormant_rp2xxx.h

#define USER_IDLE_ALARM_NUM (0u)

extern pw_wake_reason_t wake_reason;
extern volatile bool power_sleep_enabled;

void user_idle_callback();
void set_user_idle_timer();

#endif /* PW_POWER_PICO_H */
