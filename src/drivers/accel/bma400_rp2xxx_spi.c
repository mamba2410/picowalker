#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "accel_pico_bma400.h"
#include "gpio_interrupts_pico.h"

static spi_inst_t *accel_spi;
static int32_t prev_steps;

#define ACCEL_STEP_COUNT_SETTINGS_LEN 24
static const uint8_t NON_WRIST_OPTIMAL_SETTINGS[ACCEL_STEP_COUNT_SETTINGS_LEN] = {
    0x59,
    1, 50, 120, 230, 135, 0, 132,
    108, 156, 117, 100, 126, 170, 12, 12, 74, 160, 0, 0, 12, 60, 240, 1, 0
};

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

// Sends a command to reg CMD to perform, mainly used for software reset
static void bma400_send_cmd(uint8_t cmd) {
    uint8_t buf[4];

    // Check if 
    do {
        buf[0] = ACCEL_REG_STATUS;
        pw_accel_read_spi(buf, 1);
    } while( (buf[0] & REG_STATUS_CMD_READY) != REG_STATUS_CMD_READY);

    buf[0] = ACCEL_REG_CMD;
    buf[1] = cmd;
    pw_accel_write_spi(buf, 2);
}

void pw_accel_reset_int() {
    uint8_t buf[4];
    buf[0] = ACCEL_REG_STATUS;
    pw_accel_read_spi(buf, 1);

    printf("[Debug] Accel status byte: 0x%02x\n", buf[0]);

    buf[0] = ACCEL_REG_INT_STAT0;
    pw_accel_read_spi(buf, 3);
    printf("[Debug] INT_STAT0: 0x%02x, 0x%02x, 0x%02x\n", buf[0], buf[1], buf[2]);
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
    printf("[Debug] Accel has %u steps\n", steps);
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

    // Dummy read after power on to put into SPI4 mode (from I2C mode)
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);

    // Software-reset the chip
    bma400_send_cmd(REG_CMD_SOFTRESET);

    // Delay for reset, unknown time
    sleep_ms(2);

    // Dummy read again to set us back into SPI4 mode
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);

    // Read chip ID to make sure comms are ok
    buf[0] = ACCEL_REG_CHIPID;
    pw_accel_read_spi(buf, 1);

    if(buf[0] != CHIP_ID) {
        printf("[ERROR] Couldn't establish accel comms\n");
        return -1;
    }

    // Set up interrupts to be active low, open drain
    // INT2 is not used
    buf[0] = ACCEL_REG_INT12_IO_CTRL;
    buf[1] = REG_INT12_IO_CTRL_INT1_OD_OPEN_DRAIN
           | REG_INT12_IO_CTRL_INT1_LVL_ACTIVE_LOW
           | REG_INT12_IO_CTRL_INT2_OD_OPEN_DRAIN
           | REG_INT12_IO_CTRL_INT2_LVL_ACTIVE_LOW;
    pw_accel_write_spi(buf, 2);

    // Enable interrupts on step taken
    buf[0] = ACCEL_REG_INT_CONFIG1;
    buf[1] = REG_INT_CONFIG1_STEP_CNT_INT_EN;
    pw_accel_write_spi(buf, 2);

    // Map interrupts to INT1
    // Multi-write to INT2_MAP, INT2_MAP and INT12_MAP sequentially
    buf[0] = ACCEL_REG_INT1_MAP;
    buf[1] = 0; // Nothing of interest for INT1
    buf[2] = 0; // Nothing of interest for INT2
    buf[3] = REG_INT12_MAP_STEP_INT1; // Map step interrupt to pin 1
    //pw_accel_write_spi(buf, 4);

    // Disable ability to read FIFOs, apparently saves 100nA
    buf[0] = ACCEL_REG_FIFO_PWR_CONFIG;
    buf[1] = REG_FIFO_PWR_CONFIG_READ_DISABLE;
    pw_accel_write_spi(buf, 2);
    
    // Now that interrupts are configured, we set up interrupt pin
    gpio_init(ACCEL_INT_PIN);
    gpio_pull_up(ACCEL_INT_PIN);
    //gpio_set_irq_enabled_with_callback(ACCEL_INT_PIN, GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);

    prev_steps = 0;

    pw_accel_reset_int();

    pw_accel_write_spi(NON_WRIST_OPTIMAL_SETTINGS, ACCEL_STEP_COUNT_SETTINGS_LEN);

    return 0;
}

