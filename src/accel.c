#include <stdint.h>
#include <stdbool.h>

#include "accel.h"
#include "globals.h"
#include "utils.h"

void pw_accel_process_steps() {
    uint32_t new_steps = pw_accel_get_new_steps();
    new_steps += health_data_cache.steps_this_watt;
    health_data_cache.steps_this_watt = new_steps%20;
    uint16_t new_watts = new_steps/20;

    health_data_cache.today_steps += new_steps;
    if(health_data_cache.today_steps > TODAY_STEPS_MAX) health_data_cache.today_steps = TODAY_STEPS_MAX;

    health_data_cache.current_watts += new_watts;
    if(health_data_cache.current_watts > CURRENT_WATTS_MAX) health_data_cache.current_watts = CURRENT_WATTS_MAX;

}

