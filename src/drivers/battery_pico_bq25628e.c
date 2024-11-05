#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "battery_pico_bq25628e.h"
#include "../picowalker-defs.h"

void pw_battery_int() {

}

void pw_battery_init() {

    i2c_init(PMIC_I2C, PMIC_I2C_SPEED_KHZ*1000);
    gpio_set_function(PMIC_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PMIC_I2C_SCL_PIN, GPIO_FUNC_I2C);
    // hardware pull-ups

    // init things
    uint8_t buf[4] = {0};

    // Check for part number
    i2c_write_blocking(PMIC_I2C, PMIC_I2C_ADDRESS, buf, 1, false);
    i2c_read_blocking(PMIC_I2C, PMIC_I2C_ADDRESS, buf, 1, false);

    printf("BQ25628E part info: 0x%02x\n", buf[0]);
}

pw_battery_status_t pw_battery_get_status() {
    // TODO: Read voltage over I2C
    // TODO: Read status over I2C (charging, fault, etc.)
    // TODO: Convert to percent. LUT?
}

