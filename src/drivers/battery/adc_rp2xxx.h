#ifndef PW_DRIVER_BATTERY_SIMPLE_H
#define PW_DRIVER_BATTERY_SIMPLE_H

// Pico Library
#include "hardware/adc.h"

// Picowalker
#include "picowalker_structures.h"

// Function declarations
pw_power_status_t pw_power_get_status();
void pw_battery_shutdown();
void pw_power_start_measurement();

#endif  /* PW_DRIVER_BATTERY_SIMPLE_H */