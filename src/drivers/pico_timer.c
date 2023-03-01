#include <stdint.h>

#include "pico/time.h"

uint64_t pw_now_us() {
    absolute_time_t now = get_absolute_time();
    return now._private_us_since_boot;
}
