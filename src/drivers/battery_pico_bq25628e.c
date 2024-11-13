#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdio.h>

#include "battery_pico_bq25628e.h"
#include "../picowalker-defs.h"
#include "gpio_interrupts_pico.h"

//#define I2C_READ_MASK   (0x80)
#define I2C_READ_MASK   (0x00)
#define I2C_WRITE_MASK  (0x00)

static const char* CHARGE_STATUS_STRINGS[4] = {
    "not charging",
    "trickle, pre-charge or fast charge",
    "taper charge (CV mode)",
    "top-off timer active charge",
};

static char* TS_STAT_STRINGS[] = {
    "TS_NORMAL",
    "TS_COLD",
    "TS_HOT",
    "TS_COOL",
    "TS_WARM",
    "TS_PRECOOL",
    "TS_PREWARM",
    "BIAS_FAULT",
};

static char* ADC_REG_NAMES[] = {
    "IBUS",
    "IBAT",
    "VBUS",
    "VPMID",
    "VBAT",
    "VSYS",
    "TS",
    "TDIE",
};

static char* CHARGER_FLAG_0_STRINGS[] = {"WATCHDOG", "SAFETY_TMR", "VINDPM", "IINDPM", "VSYS", "TREG", "ADC_DONE"};
static char* CHARGER_FLAG_1_STRINGS[] = {"VBUS", "", "", "CHG", "", "", "", ""};
static char* FAULT_FLAG_STRINGS[] = { "TS", "", "", "TSHUT", "", "SYS_FAULT", "BAT_FAULT", "VBUS_FAULT"};
static char* CHARGER_STATUS_0_STRINGS[] = {"WATCHDOG", "SAFETY_TMR", "VINDPM", "IINDPM", "VSYS", "TREG", "ADC_DONE"} ;
static char* FAULT_STATUS_0_STRINGS[] = {"", "", "", "TSHUT", "", "SYS_FAULT", "BAT_FAULT", "VBUS_FAULT"};

static uint8_t ADC_SHIFTS[] = { 1, 2, 2, 2, 1, 1, 0, 0 };

static volatile bool adc_done;

static void pw_pmic_read_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) { return; }
    i2c_write_blocking(PMIC_I2C, PMIC_I2C_ADDRESS, &reg, 1, true);
    i2c_read_blocking(PMIC_I2C, PMIC_I2C_ADDRESS, buf, len, false);
}

static void pw_pmic_write_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) return;

    uint8_t buf2[8];
    buf2[0] = reg;
    for(uint8_t i = 1; i <= len; i++)
        buf2[i] = buf[i-1];

    i2c_write_blocking(PMIC_I2C, PMIC_I2C_ADDRESS | I2C_WRITE_MASK, buf2, len+1, false);
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
        printf("[Info] Interrupt flags: CHARGER_FLAGS_0: 0x%02x; CHARGER_FLAGS_1: 0x%02x; FAULT_FLAGS_0: 0x%02x\n", buf[0], buf[1], buf[2]);
        printf("[Info]     CHARGER_FLAGS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(buf[0] & 1)
                printf("%s |", CHARGER_FLAG_0_STRINGS[i]);
            buf[0] >>= 1;
        }

        printf("\n[Info]     CHARGER_FLAGS_1: ");
        for(size_t i = 0; i < 8; i++) {
            if(buf[1] & 1)
                printf("%s |", CHARGER_FLAG_1_STRINGS[i]);
            buf[1] >>= 1;
        }

        printf("\n[Info]     FAULT_FLAGS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(buf[2] & 1)
                printf("%s |", FAULT_FLAG_STRINGS[i]);
            buf[2] >>= 1;
        }
        printf("\n");

        adc_done = (buf[0] & REG_CHARGER_FLAG_0_ADC_DONE_FLAG) ==  REG_CHARGER_FLAG_0_ADC_DONE_FLAG;

        // Read status registers
        pw_pmic_read_reg(REG_CHARGER_STATUS_0, buf, 3);
        printf("[Info] Charger status 0: 0x%02x; Charger status 1: 0x%02x, FAULT status 0: 0x%02x\n", buf[0], buf[1], buf[2]);
        printf("[Info]     CHARGER_STATUS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(buf[0] & 1)
                printf("%s |", CHARGER_STATUS_0_STRINGS[i]);
            buf[0] >>= 1;
        }
        printf("\n[Info]     CHARGER_STATUS_1.CHG_STAT: %s\n", CHARGE_STATUS_STRINGS[(buf[1]&REG_CHARGER_STATUS_1_CHG_STAT_MSK)>>3]);
        printf("[Info]     FAULT_STATUS_0.TS_STAT: %s\n", TS_STAT_STRINGS[buf[2]&0x07]);
        printf("[Info]     FAULT_STATUS_0:");

        buf[2] >>= 3;
        for(size_t i = 3; i < 8; i++) {
            if(buf[2] & 1)
                printf("%s |", FAULT_STATUS_0_STRINGS[i]);
            buf[2] >>= 1;
        }
        printf("\n");


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

    uint8_t buf[24] = {0};

    // Check for part number
    pw_pmic_read_reg(REG_PART_INFO, buf, 3);
    printf("[Info] BQ25628E part info: 0x%02x\n", buf[0]);

    // Read `FLAG` registers to clear interrupts
    // (possibly) important to clear "power-on" interrupt
    pw_pmic_read_reg(REG_CHARGER_FLAG_0, buf, 3);

    // Set up ADC targets for conversion
    // Allow: VBAT, IBAT, VBUS, IBUS, VSYS
    // Probably just allow VBAT, VSYS in prod
    //buf[0] = REG_ADC_FUNCTION_DISABLE_0_VPMID |
    //    REG_ADC_FUNCTION_DISABLE_0_TDIE |
    //    REG_ADC_FUNCTION_DISABLE_0_TS ;
    //pw_pmic_write_reg(REG_ADC_FUNCTION_DISABLE_0, buf, 1);

    // Set ADC to one-shot mode, don't enable
    buf[0] = REG_ADC_CONTROL_ADC_AVG_DISABLED |
             REG_ADC_CONTROL_ADC_RATE_CONTINUOUS |
             REG_ADC_CONTROL_ADC_SAMPLE_9_BIT | // default
             REG_ADC_CONTROL_ADC_EN_ENABLED; // don't enable yet
    uint8_t my_reg = buf[0];
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);

    printf("[Info] Set reg to 0x%02x, read back 0x%02x\n", my_reg, buf[0]);

    // Enable ADC
    //pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
    //buf[0] |= REG_ADC_CONTROL_ADC_EN_ENABLED;
    //pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    // Just for fun
    pw_pmic_read_reg(REG_CHARGER_STATUS_0, buf, 3);
    printf("[Info] BQ25628E charge status: %s\n", CHARGE_STATUS_STRINGS[(buf[1]>>3)&0x03]);

    // TODO: For testing
    //pw_power_get_battery_status();

    debug_read_pmic();


    // TODO: Set REG_CHARGER_CONTROL_3.IBAT_PK to 0x0 for 1.5A discharge limit
    // TODO: Charge current limit default 320mA, can reprogram. See REG_CHARGE_CURRENT_LIMIT
    // TODO: VSYSMIN is 3520mV, can change with REG_MINIMAL_SYSTEM_VOLTAGE

}

void debug_read_pmic() {
    uint16_t buf16[8];
    for(size_t i = 0; i < 8; i++)
        buf16[i] = 0;
    pw_pmic_read_reg(REG_IBUS_ADC, buf16, 16);

    printf("[Info] ADC values:\n");
    buf16[0] >>= ADC_SHIFTS[0];
    buf16[1] >>= ADC_SHIFTS[1];
    printf("\t++IBUS: 0x%04x\n", buf16[0]);
    printf("\t++IBAT: 0x%04x\n", buf16[1]);
    if(buf16[0] > 0x3fff) buf16[0] = (buf16[0]^0x7fff) + 1;
    if(buf16[1] > 0x1fff) buf16[1] = (buf16[1]^0x3fff) + 1;
    printf("\tIBUS: %f mA\n", (float)(buf16[0]/*>>ADC_SHIFTS[0]*/) / (float)(0x7d0) * 4000.0);
    printf("\tIBAT: %f mA\n", (float)(buf16[1]/*>>ADC_SHIFTS[1]*/) / (float)(0x3e8) * 4000.0);
    printf("\tVBUS: %f mV\n", (float)(buf16[2]>>ADC_SHIFTS[2]) / (float)(0x11b6) * 18000.0);
    printf("\tVPMID: %f mV\n", (float)(buf16[3]>>ADC_SHIFTS[3]) / (float)(0x11b6) * 18000.0);
    printf("\tVBAT: %f mV\n", (float)(buf16[4]>>ADC_SHIFTS[4]) / (float)(0xaf0) * 5572.0);
    printf("\tVSYS: %f mV\n", (float)(buf16[5]>>ADC_SHIFTS[5]) / (float)(0xaf0) * 5572.0);
    printf("\tTDIE: %f C\n", (float)(buf16[7]>>ADC_SHIFTS[7]) / (float)(0x118) * 140.0);
}

pw_battery_status_t pw_power_get_battery_status() {
    pw_battery_status_t bs = {.percent = 100, .flags = 0x00};
    uint8_t buf[4];

    // Read battery level over I2C
    // Set ADC enabled with REG_ADC_CONTROL
    //pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
    //buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
    //buf[0] |= REG_ADC_CONTROL_ADC_EN_ENABLED;
    //pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    // Start an ADC conversion (done on enable?) and spin while waiting on an interrupt to say its done
    //while(!adc_done);

    // Read ADC registers that we're interested in 
    buf[0] = buf[1] = 0;
    //pw_pmic_read_reg(REG_VBAT_ADC, buf, 1);
    pw_pmic_read_reg(REG_VBAT_ADC, buf, 1);
    uint16_t vbat = REG_VBAT_ADC_VAL(buf[0]);
    printf("[Info] Raw VBAT 0x%04x\n", *(uint16_t*)buf);

    // Convert voltage to battery percentage
    // TODO: Some form of LUT
    // Value here is 0x000 - 0xaf0, max resolution of 1.99mV
    float vbat_f = ((float)vbat / (float)0xaf0) * 5572.0;
    printf("[Info] VBAT: %f mV\n", vbat_f);

    // Read status over I2C (charging, fault, etc.)
    // Get REG_CHARGER_STATUS_1.CHG_STAT
    // Get REG_FAULT_STATUS_0 and OR everything together (maybe ignore TS_STAT temperature zones?)

    // Disable ADC again
    //pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
    //buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
    //buf[0] |= REG_ADC_CONTROL_ADC_EN_DISABLED;
    //pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

    return bs;
}

