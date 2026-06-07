#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdio.h>

#include "board_resources.h"
#include "ft3x68_i2c_rp2xxx.h"

static void ft3x68_read_reg(ft3x68_register_t reg, uint8_t *buf, size_t len) {
    i2c_write_blocking(BAT_I2C_HW, FT3X68_I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(BAT_I2C_HW, FT3X68_I2C_ADDR, buf, len, false);
}


static void ft3x68_write_reg(ft3x68_register_t reg, uint8_t *buf, size_t len) {

    uint8_t buf2[8];
    buf2[0] = reg;
    for(uint8_t i = 1; i <= len; i++)
        buf2[i] = buf[i-1];

    i2c_write_blocking(BAT_I2C_HW, FT3X68_I2C_ADDR, buf2, len+1, false);
}


ft3x68_info_t ft3x68_get_part_info() {
    /*
     * Some serious type punning going on.
     * Annoyingly, the registers are laid out in a BE way and not all of the
     * constituent bytes are adjacent so can't just do a bulk read then
     * swap endianness.
     */
    uint8_t buf[9];

    ft3x68_read_reg(FT3X68_REG_ID_G_CIPHER_LOW, &buf[0], 1);
    ft3x68_read_reg(FT3X68_REG_ID_G_CIPHER_MID, &buf[1], 1);
    ft3x68_read_reg(FT3X68_REG_ID_G_CIPHER_HIGH, &buf[2], 1);
    buf[3] = 0;

    ft3x68_read_reg(FT3X68_REG_ID_G_LIB_VERSION_L, &buf[4], 1);
    ft3x68_read_reg(FT3X68_REG_ID_G_LIB_VERSION_H, &buf[5], 1);

    ft3x68_read_reg(FT3X68_REG_ID_G_FIRMID, &buf[6], 1);

    ft3x68_read_reg(FT3X68_REG_ID_G_FOCALTECH_ID, &buf[7], 1);

    ft3x68_read_reg(FT3X68_REG_ID_G_RELEASE_CODE_ID, &buf[8], 1);

    return *(ft3x68_info_t*)buf;
}


ft3x68_point_t ft3x68_get_point(uint8_t n) {
    (void)n;
    ft3x68_point_t point = {0};
    return point;
}


void ft3x68_peripheral_init() {
    board_i2c_init();

    gpio_init(TOUCH_RESET_PIN);
    gpio_set_dir(TOUCH_RESET_PIN, GPIO_OUT);
    gpio_put(TOUCH_RESET_PIN, 1);

    gpio_init(TOUCH_INT_PIN);
    gpio_set_dir(TOUCH_INT_PIN, GPIO_IN);
    gpio_pull_up(TOUCH_INT_PIN);

}

void ft3x68_reset() {
    gpio_put(TOUCH_RESET_PIN, 1);
    // Min 3 ms from RESETB going high to accepting a reset signal
    sleep_ms(10);
    gpio_put(TOUCH_RESET_PIN, 0);
    // Min 5 ms to acknowledge a reset
    sleep_ms(10);
    gpio_put(TOUCH_RESET_PIN, 1);
    // Min 70 ms until on
    sleep_ms(100);
}


void ft3x68_sleep() {
    uint8_t buf[] = {FT3X68_POWER_MODE_HIBERNATE};
    ft3x68_write_reg(FT3X68_REG_ID_G_PMODE, buf, 1);
}


void pw_touch_init() {
    ft3x68_peripheral_init();
    ft3x68_reset();
    ft3x68_info_t info = ft3x68_get_part_info();
    printf("[Debug] TP chip ID: 0x%06lx\n", info.cipher);
    ft3x68_sleep();
}

