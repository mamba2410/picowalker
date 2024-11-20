#include <stdint.h>

#include "pico/time.h"
#include "pico/types.h"

uint64_t pw_now_us() {
    absolute_time_t now = get_absolute_time();
    // this is mega hacky, but I don't know how else to get an
    // absolute time stamp in us as a u64
    //return now._private_us_since_boot;
    return (uint64_t)now;
}

void pw_timer_delay_ms(uint64_t ms) {
    sleep_ms(ms);
}

void pw_ir_delay_ms(uint64_t ms) {
    pw_timer_delay_ms(ms);
}

