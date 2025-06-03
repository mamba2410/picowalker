//#include <stdint.h>
//#include <stdlib. h>

//#include <stdio.h>

//#include "hardware/spi.h"
//#include "hardware/gpio.h"

#include "pico/time.h"
#include "accel_rp2350_qmi8658a.h"
//#include "gpio_interrupts_pico.h"

#define ACCEL_ADDRESS_LOW 0x6A
#define ACCEL_ADDRESS_HIGH 0x6B
#define ALPHA 0.9f
#define STEP_THRESHOLD 1.2f
#define STEP_TIMEOUT_MS 300

accel_config CONFIG;

float previous_filtered = 0.0f;
uint32_t last_step_time = 0;
uint32_t steps = 0;
uint16_t least_signed_bits = 0;

/********************************************************************************
Function: Writes from I2C
Parameters:
        register_address : register address to write to
        value: value to write
********************************************************************************/
void accel_i2c_write(uint8_t register_address, uint8_t value)
{
    uint8_t buffer[2] = {register_address, value};
    i2c_write_blocking(I2C, ACCEL_ADDRESS_LOW, buffer, 2, false);
}

/********************************************************************************
Function: Reads from I2C
Parameters:
        register_address : register address to read from
        buffer : buffer data to return
        length: length of buffer
********************************************************************************/
void accel_i2c_read(uint8_t register_address, uint8_t *buffer, uint16_t length)
{
    i2c_write_blocking(I2C, ACCEL_ADDRESS_LOW, &register_address, 1, true);
    i2c_read_blocking(I2C, ACCEL_ADDRESS_LOW, &buffer, length, false);
}

/********************************************************************************
Function: Reads Accelerometer to calculate new steps
Parameters:
********************************************************************************/
uint32_t pw_accel_get_new_steps()
{
    uint8_t buffer[6];
    uint16_t raw_xyz[3];
    float xyz[3];

    accel_i2c_read(ACCEL_X_LOW, buffer, 6);
    raw_xyz[0] = (short)((unsigned short)(buffer[1] << 8) | (buffer[0]));
    raw_xyz[1] = (short)((unsigned short)(buffer[3] << 8) | (buffer[2]));
    raw_xyz[2] = (short)((unsigned short)(buffer[5] << 8) | (buffer[4]));

    xyz[0] = (raw_xyz[0] * ONE_G) / least_signed_bits;
    xyz[1] = (raw_xyz[1] * ONE_G) / least_signed_bits;
    xyz[2] = (raw_xyz[2] * ONE_G) / least_signed_bits;

    float magnitude = sqrtf(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
    float low_pass_filter = 0.9f * previous_filtered (1.0f - 0.9f) * magnitude;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (previous_filtered < STEP_THRESHOLD && low_pass_filter >= STEP_THRESHOLD)
    {
        if ((current_time - last_step_time) > STEP_TIMEOUT_MS)
        {
            steps++;
            last_step_time = current_time;
        }
    }
    previous_filtered = low_pass_filter;    
}

/********************************************************************************
Function: Initializes Accelerometer
Parameters:
********************************************************************************/
void pw_accel_init()
{
    accel_i2c_write(ACCEL_CONTROL1, 0x60);
    CONFIG.input_selection = 0x01; // Accel Enabled
    CONFIG.range = ACCEL_RANGE_8G;
    CONFIG.output = ACCEL_OUTPUT_1000Hz;

    // apply configuration
    unsigned char control;
    enum accel_range range = CONFIG.range;
    switch (range)
    {
        case ACCEL_RANGE_2G:
            least_signed_bits = (1 << 14);
            break;
        case ACCEL_RANGE_4G:
            least_signed_bits = (1 << 13);
            break;
        case ACCEL_RANGE_8G:
            least_signed_bits = (1 << 12);
            break;
        case ACCEL_RANGE_16G:
            least_signed_bits = (1 << 11);
            break;
        default:
            range = ACCEL_RANGE_8G;
            least_signed_bits = (1 << 12);
    }
    // TODO: Review later if this need to be corrected...
    control = (unsigned char)range | (unsigned char)CONFIG.output; // ST Disabled
    accel_i2c_write(ACCEL_CONTROL2, control);
    accel_i2c_read(ACCEL_CONTROL5, &control, 1);
    control &= 0xF0;
    control |= A_LOW_PASS_FILTER_MODE_3;
    control |= 0x01;
    //control = 0x00;
    accel_i2c_write(ACCEL_CONTROL5, control);
    accel_i2c_write(ACCEL_CONTROL7, CONFIG.input_selection & 0x0F); // Review?
}

