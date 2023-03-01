#ifndef PW_TIMER_H
#define PW_TIMER_H

#include <stdint.h>

extern uint64_t pw_now_us();
extern void pw_timer_delay_ms(uint64_t ms);

#endif /* PW_TIMER_H */