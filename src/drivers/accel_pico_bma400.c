#include <stdint.h>
#include <stdlib.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "accel_pico_bma400.h"

static spi_inst_t *accel_spi;
static int32_t prev_steps;

static void pw_accel_cs_enable() {
    gpio_put(ACCEL_CS_PIN, 0);
}

static void pw_accel_cs_disable() {
    gpio_put(ACCEL_CS_PIN, 1);
}

static void pw_accel_read_spi(uint8_t *buf, size_t len) {
    buf[0] |= ACCEL_READ_MASK;
    pw_accel_cs_enable();
    spi_write_blocking(accel_spi, buf, 1);
    spi_read_blocking(accel_spi, 0, buf, len+1);    // first byte is dummy, data starts at byte 2
    pw_accel_cs_disable();

    for(size_t i = 0; i < len; i++) {
        buf[i] = buf[i+1];
    }

}

static void pw_accel_write_spi(uint8_t *buf, size_t len) {
    buf[0] &= ACCEL_WRITE_MASK;
    pw_accel_cs_enable();
    spi_write_blocking(accel_spi, buf, len);
    pw_accel_cs_disable();
}

void pw_accel_read_accel(uint8_t *buf, size_t len) {
    buf[0] = ACCEL_REG_STATUS;
    pw_accel_read_spi(buf, 1);
    uint8_t power_mode = (buf[0]>>1)&0x03;
    if(power_mode == 0x00) {
        buf[0] = ACCEL_REG_ACC_CONFIG0;
        buf[1] = 0x02;
        pw_accel_write_spi(buf, 2);
        sleep_ms(2);
    }

    buf[0] = ACCEL_REG_ACC_X_LSB;
    pw_accel_read_spi(buf, 6);      // all at once
}

static void pw_accel_read_steps(uint8_t *buf) {
    buf[0] = ACCEL_REG_STATUS;
    pw_accel_read_spi(buf, 1);
    uint8_t power_mode = (buf[0]>>ACCEL_POWER_OFFSET)&ACCEL_POWER_MASK;
    if(power_mode == ACCEL_POWER_SLEEP) {
        buf[0] = ACCEL_REG_ACC_CONFIG0;
        buf[1] = ACCEL_POWER_NORMAL;
        pw_accel_write_spi(buf, 2);
        sleep_ms(2);    // 1500 us
    }

    buf[0] = ACCEL_REG_STEP_CNT_0;
    pw_accel_read_spi(buf, 4);      // all at once
}




/*
void pw_accel_test() {
    uint8_t buf[8];


    // if you want interrupt on int pin 1
    buf[0] = ACCEL_REG_INT1_MAP;
    pw_accel_read_spi(buf, 3);

    buf[1] = buf[2] | 0x01;
    buf[0] = ACCEL_REG_INT12_MAP;
    pw_accel_write_spi(buf, 2);

    buf[1] = STEP_CNT_INT_EN;
    buf[0] = ACCEL_REG_INT_CONFIG1;
    pw_accel_write_spi(buf, 2);

    while(1) {
        pw_accel_read_accel(buf, 6);
        int16_t x_accel = (buf[0]&0xff) + ((buf[1]&0x0f)<<8);
        if(x_accel > 2047) x_accel -= 4096;

        pw_accel_read_steps(buf);
        uint32_t steps = (buf[2]<<16) | (buf[1]<<8) | (buf[0]);

        sleep_ms(1000);

    }
}
*/

uint32_t pw_accel_get_new_steps() {
    uint8_t buf[5];
    pw_accel_read_steps(buf);
    int32_t steps = (buf[2]<<16) | (buf[1]<<8) | (buf[0]);
    int32_t new_steps = steps - prev_steps;
    prev_steps = steps;
    if(new_steps < 0) return 0;
    else return (uint32_t)new_steps;
}

int8_t pw_accel_init() {
    accel_spi = spi0;

    gpio_init(ACCEL_CS_PIN);
    gpio_set_dir(ACCEL_CS_PIN, GPIO_OUT);
    pw_accel_cs_disable();

    //spi_init(accel_spi, ACCEL_SPI_SPEED);
    //// inst, bits, polarity, phase, endian
    //spi_set_format(accel_spi, 8, 1, 1, SPI_MSB_FIRST);

    //gpio_set_function(ACCEL_SCL_PIN, GPIO_FUNC_SPI);
    //gpio_set_function(ACCEL_MOSI_PIN, GPIO_FUNC_SPI);
    //gpio_set_function(ACCEL_MISO_PIN, GPIO_FUNC_SPI);


    uint8_t buf[8];
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);    // dummy read
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);

    if(buf[0] != CHIP_ID) {
        printf("[ERROR] Couldn't establish accel comms\n");
        return -1;
    }

    buf[1] = STEP_CNT_INT_EN;
    buf[0] = ACCEL_REG_INT_CONFIG1;
    pw_accel_write_spi(buf, 2);

    prev_steps = 0;

    return 0;
}

