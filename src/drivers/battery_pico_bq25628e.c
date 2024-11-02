#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "battery_pico_bq25628e.h"
#include "../picowalker-defs.h"

void pw_battery_init() {
    // TODO: start i2c
    // TODO: any init things
}

pw_battery_status_t pw_battery_get_status() {
    // TODO: Read voltage over I2C
    // TODO: Read status over I2C (charging, fault, etc.)
    // TODO: Convert to percent. LUT?
}

