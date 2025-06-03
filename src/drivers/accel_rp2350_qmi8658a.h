#ifndef PW_ACCEL_RP2350_QMI8658A_H
#define PW_ACCEL_RP2350_QMI8658A_H

#include <stdlib.h>
#include <stdio.h>

#include "i2c_rp2350.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846f)
#endif

#ifndef ONE_G
#define ONE_G (9.807f)
#endif

enum accel_register
{
    ACCEL_CONTROL1 = 2, // General & Power Management Mode
    ACCEL_CONTROL2 = 3, // Accelerometer Control 
    ACCEL_CONTROL5 = 6, // Data Processing Settings
    ACCEL_CONTROL7 = 8, // Sensor Enabled Status
    ACCEL_X_LOW = 53,
    ACCEL_X_HIGH,
    ACCEL_Y_LOW,
    ACCEL_Y_HIGH,
    ACCEL_Z_LOW,
    ACCEL_Z_HIGH,
    ACCEL_I2C_STATUS = 110
};

enum accel_range
{
    ACCEL_RANGE_2G = 0x00 << 4,
    ACCEL_RANGE_4G = 0x01 << 4,
    ACCEL_RANGE_8G = 0x02 << 4,
    ACCEL_RANGE_16G = 0x03 << 4
};

enum accel_output
{
    ACELL_OUTPUT_8000Hz = 0x00,
    ACCEL_OUTPUT_4000Hz = 0x01,
    ACCEL_OUTPUT_2000Hz = 0x02,
    ACCEL_OUTPUT_1000Hz = 0x03,
    ACCEL_OUTPUT_500Hz = 0x04,
    ACCEL_OUTPUT_250Hz = 0x05,
    ACCEL_OUTPUT_125Hz = 0x06,
    ACCEL_OUTPUT_62Hz = 0x07,
    ACCEL_OUTPUT_31Hz = 0x08,
    ACCEL_OUTPUT_LOW_POWER_128Hz = 0x0C,
    ACCEL_OUTPUT_LOW_POWER_21Hz = 0x0D,
    ACCEL_OUTPUT_LOW_POWER_11Hz = 0x0E,
    ACCEL_OUTPUT_LOW_POWER_3Hz = 0x0F
};

enum accel_low_pass_filter_mode
{
    A_LOW_PASS_FILTER_MODE_0 = 0x00 << 1,
    A_LOW_PASS_FILTER_MODE_1 = 0x01 << 1,
    A_LOW_PASS_FILTER_MODE_2 = 0x02 << 1,
    A_LOW_PASS_FILTER_MODE_3 = 0x03 << 1,
    G_LOW_PASS_FILTER_MODE_0 = 0x00 << 5,
    G_LOW_PASS_FILTER_MODE_1 = 0x01 << 5,
    G_LOW_PASS_FILTER_MODE_2 = 0x02 << 5,
    G_LOW_PASS_FILTER_MODE_3 = 0x03 << 5,

};

enum accel_filter
{
    ACCEL_DISABLE,
    ACCEL_ENABLE
};


typedef struct
{
    unsigned char input_selection;
    enum accel_range range;
    enum accel_output output;
} accel_config;

#endif /* PW_ACCEL_RP2350_QMI8658A_H */