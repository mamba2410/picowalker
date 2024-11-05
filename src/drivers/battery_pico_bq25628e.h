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
    REG_0x02_CHARGE_CURRENT_LIMIT = 0x02,
    REG_0x04_CHARGE_VOLTAGE_LIMIT = 0x04,
    REG_0x06_INPUT_CURRENT_LIMIT = 0x06,
    REG_0x08_INPUT_VOLTAGE_LIMIT = 0x08,
    REG_0x0E_MINIMAL_SYSTEM_VOLTAGE = 0x0e,
    REG_0x1D_CHARGER_STATUS_0 = 0x1d,
    REG_0x1E_CHARGER_STATUS_1 = 0x1e,
    REG_0x38_PART_INFO = 0x38,
};

#endif /* BATTERY_PICO_B25628E_H */

