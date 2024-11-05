#ifndef BATTERY_PICO_B25628E_H
#define BATTERY_PICO_B25628E_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BAT_INT_PIN 27  // GP27, phys 32
#define BAT_CE_PIN  28  // GP28, phys 34

#define PMIC_I2C_SCL_PIN 21  // GP21, phys 27
#define PMIC_I2C_SDA_PIN 20  // GP20, phys 26

#define PMIC_I2C_SPEED_KHZ 100 // Supports 400 and 1000 also but needs more timing requirements
#define PMIC_I2C_ADDRESS 0x6a

#define PMIC_I2C i2c0

enum bq25628e_reg_address_e {
    REG0x38_PART_INFO = 0x38,
};

#endif /* BATTERY_PICO_B25628E_H */

