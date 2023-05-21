#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "accel_bma400.h"

static spi_inst_t *accel_spi;

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

void pw_read_accel(uint8_t *buf, size_t len) {
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

void pw_accel_read_steps(uint8_t *buf, size_t len) {
    buf[0] = ACCEL_REG_STATUS;
    pw_accel_read_spi(buf, 1);
    uint8_t power_mode = (buf[0]>>1)&0x03;
    if(power_mode == 0x00) {
        buf[0] = ACCEL_REG_ACC_CONFIG0;
        buf[1] = 0x02;
        pw_accel_write_spi(buf, 2);
        sleep_ms(2);
    }

    buf[0] = ACCEL_REG_STEP_CNT_0;
    pw_accel_read_spi(buf, 4);      // all at once
}


void pw_accel_init() {
    accel_spi = spi0;

    pw_accel_cs_disable();
    spi_init(accel_spi, 1000*1000);
    // inst, bits, polarity, phase, endian
    spi_set_format(accel_spi, 8, 1, 1, SPI_MSB_FIRST);

    gpio_set_function(ACCEL_SCL_PIN, GPIO_FUNC_SPI);
    gpio_set_function(ACCEL_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(ACCEL_MISO_PIN, GPIO_FUNC_SPI);

    gpio_init(ACCEL_CS_PIN);
    gpio_put(ACCEL_CS_PIN, 1);
    gpio_set_dir(ACCEL_CS_PIN, GPIO_OUT);

    uint8_t buf[16];
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);    // dummy read
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);

    printf("chip id: 0x%02x\n", buf[0]);

}

void pw_accel_test() {
    uint8_t buf[8];


    /*
    // if you want interrupt on int pin 1
    buf[0] = ACCEL_REG_INT1_MAP;
    pw_accel_read_spi(buf, 3);

    buf[1] = buf[2] | 0x01;
    buf[0] = ACCEL_REG_INT12_MAP;
    pw_accel_write_spi(buf, 2);
    */

    buf[1] = STEP_CNT_INT_EN;
    buf[0] = ACCEL_REG_INT_CONFIG1;
    pw_accel_write_spi(buf, 2);

    while(1) {
        pw_read_accel(buf, 6);
        int16_t x_accel = (buf[0]&0xff) + ((buf[1]&0x0f)<<8);
        if(x_accel > 2047) x_accel -= 4096;
        printf("accel x: %d\n", x_accel);

        pw_accel_read_steps(buf, 0);
        uint32_t steps = (buf[2]<<16) | (buf[1]<<8) | (buf[0]);
        char status[16];
        switch(buf[3]) {
        case 0:
            sprintf(status, "Still");
            break;
        case 1:
            sprintf(status, "Walking");
            break;
        case 2:
            sprintf(status, "Running");
            break;
        }
        printf("steps: %d\nstat: %s\n", steps, status);
        //printf("steps: %d\nstat: 0x%02x\n", steps, buf[3]);

        sleep_ms(1000);

    }
}

