#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdio.h>

#include "battery_pico_bq25628e.h"
#include "../picowalker-defs.h"
#include "gpio_interrupts_pico.h"

static const char* CHARGE_STATUS_STRINGS[4] = {
    "not charging",
    "trickle, pre-charge or fast charge",
    "taper charge (CV mode)",
    "top-off timer active charge",
};

static volatile bool adc_done;

static void pw_pmic_read_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) return;
    i2c_write_blocking(PMIC_I2C, PMIC_I2C_ADDRESS | 0x80, &reg, 1, false);
    i2c_read_blocking(PMIC_I2C, PMIC_I2C_ADDRESS, buf, len, false);
}

static void pw_pmic_write_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) return;
    i2c_write_blocking(PMIC_I2C, PMIC_I2C_ADDRESS | 0x80, &reg, 1, false);
    i2c_write_blocking(PMIC_I2C, PMIC_I2C_ADDRESS | 0x80, buf, len, false);
}

/*
 * TODO:
 * See REG_CHARGER_CONTROL_2.BATFET_CTRL for entering "ship mode" and "shutdown mode" for long-term storage if there hasn't been activity in a while.
 * /QON has something to do with waking up from ship mode
 * REG_CHARGER_CONTROL_3.BATFET_CTRL_WVBUS for power chip to cut power to MCU on adapter power. Might be useful?
 */

void pw_battery_int(uint gp, uint32_t events) {
    
    switch(gp) {
    case BAT_INT_PIN: {
        uint8_t buf[4];

        // Read `FLAG` registers to clear interrupts
        pw_pmic_read_reg(REG_CHARGER_FLAG_0, buf, 3);
        printf("[Info] Interrupt flags: CHARGER_FLAGS_0: 0x%02x; CHARGER_FLAGS_1: 0x%02x; FAULT_FLAGS_0: 0x%02x\n", buf[1], buf[2], buf[3]);

        adc_done = (buf[0] & REG_CHARGER_FLAG_0_ADC_DONE_FLAG) ==  REG_CHARGER_FLAG_0_ADC_DONE_FLAG;

        // Read status registers
        pw_pmic_read_reg(REG_CHARGER_STATUS_0, buf, 3);
        printf("[Info] Charger status 0: 0x%02x; Charger status 1: 0x%02x, FAULT status 0: 0x%02x\n", buf[0], buf[1], buf[2]);


        // Should probably do something with this info, like notify core about something like a fault, low battery, on charge
        break;
    }
    default: {
        printf("[Error] battery callback on pin %d\n", gp);
        break;
    }
    }
}

void pw_battery_init() {

    // I2C bus
    i2c_init(PMIC_I2C, PMIC_I2C_SPEED_KHZ*1000);
    gpio_set_function(PMIC_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PMIC_I2C_SCL_PIN, GPIO_FUNC_I2C);
    // hardware I2C pull-ups

    // Always allow charge, for now...
    gpio_init(BAT_CE_PIN);
    gpio_set_dir(BAT_CE_PIN, GPIO_OUT);
    gpio_put(BAT_CE_PIN, 0);

    // Set up INT pin with callback
    gpio_init(BAT_INT_PIN);
    gpio_pull_up(BAT_INT_PIN);
    gpio_set_irq_enabled_with_callback(BAT_INT_PIN, GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);

    uint8_t buf[4] = {0};

    // Check for part number
    pw_pmic_read_reg(REG_PART_INFO, buf, 3);
    printf("BQ25628E part info: 0x%02x\n", buf[0]);

    // Read `FLAG` registers to clear interrupts
    // (possibly) important to clear "power-on" interrupt
    pw_pmic_read_reg(REG_CHARGER_FLAG_0, buf, 3);

    // Set up ADC targets for conversion
    // Allow: VBAT, IBAT, VBUS, IBUS, VSYS
    // Probably just allow VBAT, VSYS in prod
    buf[0] = REG_ADC_FUNCTION_DISABLE_0_VPMID |
        REG_ADC_FUNCTION_DISABLE_0_TDIE |
        REG_ADC_FUNCTION_DISABLE_0_TS ;
    pw_pmic_write_reg(REG_ADC_FUNCTION_DISABLE_0, buf, 1);

    // Set ADC to one-shot mode, don't enable
    buf[0] = REG_ADC_CONTROL_ADC_AVG_DISABLED |
             REG_ADC_CONTROL_ADC_RATE_ONESHOT |
             REG_ADC_CONTROL_ADC_SAMPLE_9_BIT | // default
             REG_ADC_CONTROL_ADC_EN_DISABLED; // don't enable yet
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    // Just for fun
    pw_pmic_read_reg(REG_CHARGER_STATUS_0, buf, 3);
    printf("BQ25628E charge status: %s\n", CHARGE_STATUS_STRINGS[(buf[1]>>3)&0x03]);

    // TODO: For testing
    pw_power_get_battery_status();

    // TODO: Set REG_CHARGER_CONTROL_3.IBAT_PK to 0x0 for 1.5A discharge limit
    // TODO: Charge current limit default 320mA, can reprogram. See REG_CHARGE_CURRENT_LIMIT
    // TODO: VSYSMIN is 3520mV, can change with REG_MINIMAL_SYSTEM_VOLTAGE

}

pw_battery_status_t pw_power_get_battery_status() {
    pw_battery_status_t bs = {.percent = 100, .flags = 0x00};
    uint8_t buf[4];

    // Read battery level over I2C
    // Set ADC enabled with REG_ADC_CONTROL
    pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
    buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
    buf[0] |= REG_ADC_CONTROL_ADC_EN_ENABLED;
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    // Start an ADC conversion (done on enable?) and spin while waiting on an interrupt to say its done
    //while(!adc_done);

    // Read ADC registers that we're interested in 
    pw_pmic_read_reg(REG_VBAT_ADC, buf, 1);
    uint16_t vbat = REG_VBAT_ADC_VAL(buf[0]);

    // Convert voltage to battery percentage
    // TODO: Some form of LUT
    // Value here is 0x000 - 0xaf0, max resolution of 1.99mV
    float vbat_f = ((float)vbat / (float)0xaf0) * 5572.0;
    printf("[Info] VBAT: %f mV\n", vbat_f);

    // Read status over I2C (charging, fault, etc.)
    // Get REG_CHARGER_STATUS_1.CHG_STAT
    // Get REG_FAULT_STATUS_0 and OR everything together (maybe ignore TS_STAT temperature zones?)

    // Disable ADC again
    pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
    buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
    buf[0] |= REG_ADC_CONTROL_ADC_EN_DISABLED;
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    return bs;
}

