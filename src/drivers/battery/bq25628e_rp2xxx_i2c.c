#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdio.h>

#include "board_resources.h"
#include "bq25628e_rp2xxx_i2c.h"
#include "../sleep/dormant_rp2xxx.h"
#include "../interrupts/rp2xxx_gpio.h"
#include "../../picowalker-defs.h"

//#define I2C_READ_MASK   (0x80)
#define I2C_READ_MASK   (0x00)
#define I2C_WRITE_MASK  (0x00)

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
#define VBAT_ABS_MINIMUM_MV 3500.0f
#define VBAT_ABS_MAXIMUM_MV 4250.0f
#define VBAT_SAFE_MINIMUM_MV 3600.0f

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
static char log_staging[128] = "";

>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
static const char* CHARGE_STATUS_STRINGS[4] = {
    "not charging",
    "trickle, pre-charge or fast charge",
    "taper charge (CV mode)",
    "top-off timer active charge",
};

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
static const char* CHARGE_STATUS_SHORT[4] = {
    "D", "CC", "CV", "TOP",
};

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
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

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
static volatile uint8_t interrupt_flags[4];
static volatile uint8_t interrupt_status[4];
static volatile bool adc_done = false;
static volatile uint32_t adc_timeout_stamp = 0;
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
static volatile bool adc_done;
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

static void pw_pmic_read_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) { return; }
    i2c_write_blocking(BAT_I2C_HW, BAT_I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(BAT_I2C_HW, BAT_I2C_ADDR, buf, len, false);
}

static void pw_pmic_write_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) return;

    uint8_t buf2[8];
    buf2[0] = reg;
    for(uint8_t i = 1; i <= len; i++)
        buf2[i] = buf[i-1];

    i2c_write_blocking(BAT_I2C_HW, BAT_I2C_ADDR | I2C_WRITE_MASK, buf2, len+1, false);
}

/*
 * TODO:
 * See REG_CHARGER_CONTROL_2.BATFET_CTRL for entering "ship mode" and "shutdown mode" for long-term storage if there hasn't been activity in a while.
 * /QON has something to do with waking up from ship mode
 * REG_CHARGER_CONTROL_3.BATFET_CTRL_WVBUS for power chip to cut power to MCU on adapter power. Might be useful?
 */

void print_flags_and_status(uint8_t flags[3], uint8_t status[3]) {

        printf("[Info] ============\n");
        printf("[Info] Interrupt flags: CHARGER_FLAGS_0: 0x%02x; CHARGER_FLAGS_1: 0x%02x; FAULT_FLAGS_0: 0x%02x\n", flags[0], flags[1], flags[2]);
        printf("[Info]     CHARGER_FLAGS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(flags[0] & 1)
                printf("%s (%d) |", CHARGER_FLAG_0_STRINGS[i], i);
            flags[0] >>= 1;
        }

        printf("\n[Info]     CHARGER_FLAGS_1: ");
        for(size_t i = 0; i < 8; i++) {
            if(flags[1] & 1)
                printf("%s |", CHARGER_FLAG_1_STRINGS[i]);
            flags[1] >>= 1;
        }

        printf("\n[Info]     FAULT_FLAGS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(flags[2] & 1)
                printf("%s |", FAULT_FLAG_STRINGS[i]);
            flags[2] >>= 1;
        }
        printf("\n");

        // Read status registers
        printf("[Info] Charger status 0: 0x%02x; Charger status 1: 0x%02x, FAULT status 0: 0x%02x\n", status[0], status[1], status[2]);
        printf("[Info]     CHARGER_STATUS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(status[0] & 1)
                printf("%s |", CHARGER_STATUS_0_STRINGS[i]);
            status[0] >>= 1;
        }
        printf("\n[Info]     CHARGER_STATUS_1.CHG_STAT: %s\n", CHARGE_STATUS_STRINGS[(status[1]&REG_CHARGER_STATUS_1_CHG_STAT_MSK)>>3]);
        printf("[Info]     FAULT_STATUS_0.TS_STAT: %s\n", TS_STAT_STRINGS[status[2]&0x07]);
        printf("[Info]     FAULT_STATUS_0:");

        status[2] >>= 3;
        for(size_t i = 3; i < 8; i++) {
            if(status[2] & 1)
                printf("%s |", FAULT_STATUS_0_STRINGS[i]);
            status[2] >>= 1;
        }
        printf("\n");
}


/*
 * Interrupt handler from the PMIC
 * Read flags registers to determine what caused the interrupt and clear them.
 */
void pw_battery_int(uint gp, uint32_t events) {
    
    if(gp == BAT_INT_PIN) {
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD

        // Read `FLAG` registers to clear interrupts
        pw_pmic_read_reg(REG_CHARGER_FLAG_0, interrupt_flags, 3);
        pw_pmic_read_reg(REG_CHARGER_STATUS_0, interrupt_status, 3);

        // Latch up, don't reset. Otherwise interrupts before we check the flag
        // will cause things to spin forever
        if(interrupt_flags[0] & REG_CHARGER_FLAG_0_ADC_DONE_FLAG) {
=======
        uint8_t flags[4], status[4];
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

        // Read `FLAG` registers to clear interrupts
        pw_pmic_read_reg(REG_CHARGER_FLAG_0, interrupt_flags, 3);
        pw_pmic_read_reg(REG_CHARGER_STATUS_0, interrupt_status, 3);

        // Latch up, don't reset. Otherwise interrupts before we check the flag
        // will cause things to spin forever
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
        if(flags[0] & REG_CHARGER_FLAG_0_ADC_DONE_FLAG) {
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
        if(interrupt_flags[0] & REG_CHARGER_FLAG_0_ADC_DONE_FLAG) {
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
            adc_done = true;
        }

        // Debug print if any of the fault flags show up
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
        if(interrupt_flags[2] > 0) {
            // Should probably do something with this info, like notify core about something like a fault, low battery, on charge
            print_flags_and_status(interrupt_flags, interrupt_status);
=======
        if(flags[2] > 0) {
            // Should probably do something with this info, like notify core about something like a fault, low battery, on charge
            pw_pmic_read_reg(REG_CHARGER_STATUS_0, status, 3);
            print_flags_and_status(flags, status);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
        if(interrupt_flags[2] > 0) {
            // Should probably do something with this info, like notify core about something like a fault, low battery, on charge
            print_flags_and_status(interrupt_flags, interrupt_status);
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
        }

    } else {
        printf("[Error] battery callback on pin gp%d\n", gp);
    }
}


<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
void pw_battery_shutdown() {
    uint8_t val = REG_CHARGER_CONTROL_2_BATFET_CTRL_SHUTDOWN;
    pw_pmic_write_reg(REG_CHARGER_CONTROL_2, &val, 1);
}

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
void pw_battery_init() {

    board_i2c_init();

    // Disable charge for now
    gpio_init(BAT_CE_PIN);
    gpio_set_dir(BAT_CE_PIN, GPIO_OUT);
    gpio_put(BAT_CE_PIN, 1);

    // Set up INT pin with callback
    gpio_init(BAT_INT_PIN);
    gpio_pull_up(BAT_INT_PIN);
    gpio_set_irq_enabled_with_callback(BAT_INT_PIN, GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);

    uint8_t buf[24] = {0};

    // Reset the PMIC
    pw_pmic_read_reg(REG_CHARGER_CONTROL_1, buf, 1);
    buf[0] &= ~REG_CHARGER_CONTROL_1_REG_RST_MSK;
    buf[0] |= REG_CHARGER_CONTROL_1_REG_RST;
    pw_pmic_write_reg(REG_CHARGER_CONTROL_1, buf, 1);

    // Check for part number
    pw_pmic_read_reg(REG_PART_INFO, buf, 3);
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
    printf("[Info ] BQ25628E part info: 0x%02x\n", buf[0]);
=======
    printf("[Info] BQ25628E part info: 0x%02x\n", buf[0]);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    printf("[Info ] BQ25628E part info: 0x%02x\n", buf[0]);
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // Read `FLAG` registers to clear interrupts
    // (possibly) important to clear "power-on" interrupt
    pw_pmic_read_reg(REG_CHARGER_FLAG_0, buf, 3);

    // Disable the watchdog timer so that its always in host mode.
    // TODO: Probably safer to "pet" the watchdog periodically with a 
    // hardware timer but this will do for now
    // Default is 50s, can set up to 200s
    pw_pmic_read_reg(REG_CHARGER_CONTROL_0, buf, 1);
    buf[0] &= ~REG_CHARGER_CONTROL_0_WATCHDOG_MSK;
    buf[0] |= REG_CHARGER_CONTROL_0_WATCHDOG_DISABLED;
    pw_pmic_write_reg(REG_CHARGER_CONTROL_0, buf, 1);

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
    // Enable charge
    gpio_put(BAT_CE_PIN, 0);

    // Set up ADC targets for conversion
    // Allow: VBAT, VSYS
    buf[0] = REG_ADC_FUNCTION_DISABLE_0_VPMID |
        REG_ADC_FUNCTION_DISABLE_0_TDIE |
        REG_ADC_FUNCTION_DISABLE_0_IBAT |
        REG_ADC_FUNCTION_DISABLE_0_IBUS |
        REG_ADC_FUNCTION_DISABLE_0_VBUS |
        REG_ADC_FUNCTION_DISABLE_0_TS ;
    pw_pmic_write_reg(REG_ADC_FUNCTION_DISABLE_0, buf, 1);

    /*
    // Set ADC to one-shot mode, don't enable to save power
    // Have long ADC samples at first to "prime" the ADC
    buf[0] = REG_ADC_CONTROL_ADC_AVG_DISABLED |
             REG_ADC_CONTROL_ADC_RATE_ONESHOT |
             //REG_ADC_CONTROL_ADC_SAMPLE_9_BIT | // default
             //REG_ADC_CONTROL_ADC_SAMPLE_12_BIT | // extend ADC sample time to 30 ms
=======
    // Set up ADC targets for conversion
    // Allow: VBAT, IBAT, VBUS, IBUS, VSYS
    // Probably just allow VBAT, VSYS in prod
    //buf[0] = REG_ADC_FUNCTION_DISABLE_0_VPMID |
    //    REG_ADC_FUNCTION_DISABLE_0_TDIE |
    //    REG_ADC_FUNCTION_DISABLE_0_TS ;
    //pw_pmic_write_reg(REG_ADC_FUNCTION_DISABLE_0, buf, 1);
=======
    // Enable charge
    gpio_put(BAT_CE_PIN, 0);
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // Set up ADC targets for conversion
    // Allow: VBAT, VSYS
    buf[0] = REG_ADC_FUNCTION_DISABLE_0_VPMID |
        REG_ADC_FUNCTION_DISABLE_0_TDIE |
        REG_ADC_FUNCTION_DISABLE_0_IBAT |
        REG_ADC_FUNCTION_DISABLE_0_IBUS |
        REG_ADC_FUNCTION_DISABLE_0_VBUS |
        REG_ADC_FUNCTION_DISABLE_0_TS ;
    pw_pmic_write_reg(REG_ADC_FUNCTION_DISABLE_0, buf, 1);

    /*
    // Set ADC to one-shot mode, don't enable to save power
    // Have long ADC samples at first to "prime" the ADC
    buf[0] = REG_ADC_CONTROL_ADC_AVG_DISABLED |
             REG_ADC_CONTROL_ADC_RATE_ONESHOT |
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
             REG_ADC_CONTROL_ADC_SAMPLE_9_BIT | // default
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
             //REG_ADC_CONTROL_ADC_SAMPLE_9_BIT | // default
             //REG_ADC_CONTROL_ADC_SAMPLE_12_BIT | // extend ADC sample time to 30 ms
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
             REG_ADC_CONTROL_ADC_EN_DISABLED; // don't enable yet
    uint8_t my_reg = buf[0];
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    // Do a few samples to prime the ADC and stop it from instantly shutting down.
    // TODO: Do this a better way
    for(size_t i = 0; i < 3; i++) {
        pw_power_get_battery_status();
    }
    */

    // Back to normal length ADC samples
    buf[0] = REG_ADC_CONTROL_ADC_AVG_DISABLED |
             REG_ADC_CONTROL_ADC_RATE_ONESHOT |
             REG_ADC_CONTROL_ADC_SAMPLE_10_BIT | // 7.5 ms each
             REG_ADC_CONTROL_ADC_EN_DISABLED; // don't enable yet
    //my_reg = buf[0];
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    // Just for fun
    //pw_pmic_read_reg(reg_charger_status_0, buf, 3);
    //printf("[info] bq25628e charge status: %s\n", charge_status_strings[(buf[1]>>3)&0x03]);

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
    // Enable charge
    gpio_put(BAT_CE_PIN, 0);

    // Debug: For testing
    pw_power_get_battery_status();

>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    // TODO: Set REG_CHARGER_CONTROL_3.IBAT_PK to 0x0 for 1.5A discharge limit
    // TODO: Charge current limit default 320mA, can reprogram. See REG_CHARGE_CURRENT_LIMIT
    // TODO: VSYSMIN is 3520mV, can change with REG_MINIMAL_SYSTEM_VOLTAGE

}

void debug_read_pmic() {
    uint16_t buf16[8];
    for(size_t i = 0; i < 8; i++)
        buf16[i] = 0;
    pw_pmic_read_reg(REG_IBUS_ADC, (uint8_t*)buf16, 16);

    printf("[Info] ADC values:\n");
    buf16[0] >>= ADC_SHIFTS[0];
    buf16[1] >>= ADC_SHIFTS[1];
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


/*
 * Callback function called periodically by picowalker-core
 * Runs in normal context
 */
pw_battery_status_t pw_power_get_battery_status() {
    pw_battery_status_t bs = {.percent = 100, .flags = 0x00};
    uint8_t buf[8];
    uint16_t *buf16 = (uint16_t*)buf;

    // Enable the ADC and start conversion
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    if(adc_timeout_stamp == 0) {
        pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
        buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
        buf[0] |= REG_ADC_CONTROL_ADC_EN_ENABLED;
        pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);
        adc_timeout_stamp = pw_time_get_ms() + ADC_TIMEOUT_MS;
    }

    // Check why we interrupted, if at all
    //print_flags_and_status(interrupt_flags, interrupt_status);
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
    pw_pmic_read_reg(REG_ADC_CONTROL, buf, 1);
    buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
    buf[0] |= REG_ADC_CONTROL_ADC_EN_ENABLED;
    pw_pmic_write_reg(REG_ADC_CONTROL, buf, 1);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // Pet the watchdog
    // TODO: Can't rely on this in sleep mode
    //pw_pmic_read_reg(REG_CHARGER_CONTROL_0, buf, 1);
    //buf[0] &= ~REG_CHARGER_CONTROL_0_WD_RST_MSK;
    //buf[0] |= REG_CHARGER_CONTROL_0_WD_RST;
    //pw_pmic_write_reg(REG_CHARGER_CONTROL_0, buf, 1);

    // Read status registers while we wait for ADC conversion
    pw_pmic_read_reg(REG_CHARGER_STATUS_0, buf, 3);
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    uint8_t charge_status = (buf[1]>>3)&0x03;
    //printf("[Info] bq25628e charge status: %s\n", CHARGE_STATUS_STRINGS[charge_status]);
    if(charge_status != 0) {
        bs.flags |= PW_BATTERY_STATUS_FLAGS_CHARGING;
    }

    uint8_t fault_mask = 
        REG_FAULT_STATUS_0_SHUT_STAT |
        REG_FAULT_STATUS_0_SYS_FAULT_STAT |
        REG_FAULT_STATUS_0_BAT_FAULT_STAT |
        REG_FAULT_STATUS_0_VBUS_FAULT_STAT;

    uint8_t fault_status = buf[2] & fault_mask;
    if(fault_status != 0) {
        bs.flags |= PW_BATTERY_STATUS_FLAGS_FAULT;
    }

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
    // Wait for interrupt to say that ADC function is done
    while(!adc_done && (pw_time_get_ms() < adc_timeout_stamp));
    if(!adc_done) {
        printf("[Warn ] Battery ADC measurement exceeded %d ms\n", ADC_TIMEOUT_MS);
        bs.flags |= PW_BATTERY_STATUS_FLAGS_TIMEOUT;
        adc_timeout_stamp = 0;
        return bs;
    }
    uint32_t adc_conversion_time = pw_time_get_ms() - (adc_timeout_stamp - ADC_TIMEOUT_MS);
    adc_done = false;
    adc_timeout_stamp = 0;
=======
    printf("[Info] bq25628e charge status: %s\n", CHARGE_STATUS_STRINGS[(buf[1]>>3)&0x03]);
=======
    // TODO: Remove
    // This is a workaround for TinyUSB not being able to detect an unmount call on pico devices
    // If we aren't charging, we can pretend like that's good enough and assume we are unplugged
    // Side-effect is that it will enable sleep if its plugged in but not charging
    if(buf[1]>>3 == 0 && power_sleep_enabled == false){
        tud_umount_cb();
    }
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // Wait for interrupt to say that ADC function is done
    while(!adc_done && (pw_time_get_ms() < adc_timeout_stamp));
    if(!adc_done) {
        printf("[Warn ] Battery ADC measurement exceeded %d ms\n", ADC_TIMEOUT_MS);
        bs.flags |= PW_BATTERY_STATUS_FLAGS_TIMEOUT;
        adc_timeout_stamp = 0;
        return bs;
    }
    uint32_t adc_conversion_time = pw_time_get_ms() - (adc_timeout_stamp - ADC_TIMEOUT_MS);
    adc_done = false;
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    adc_timeout_stamp = 0;
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // Read ADC registers that we're interested in 
    buf[0] = buf[1] = 0;
    pw_pmic_read_reg(REG_VBAT_ADC, buf, 2);
    uint16_t raw_val = buf16[0];
    uint16_t vbat = REG_VBAT_ADC_VAL(raw_val);
    //printf("[Info] Raw VBAT 0x%04x\n", raw_val);

    // Convert voltage to battery percentage
    // TODO: Some form of LUT
    // Value here is 0x000 - 0xaf0, max resolution of 1.99mV
    float vbat_f = ((float)vbat / (float)0xaf0) * 5572.0;
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    //printf("[Log  ] VBAT: %4.0f mV\n", vbat_f);
    float percent_f = 100.0f*(vbat_f - VBAT_ABS_MINIMUM_MV)/(VBAT_ABS_MAXIMUM_MV-VBAT_ABS_MINIMUM_MV);
    bs.percent = (uint8_t)percent_f;
    if(percent_f > 105) {
        printf("[Warn ] Battery percentage above 100%%: %d%%\n", (uint8_t)percent_f);
        bs.percent = 100;
    }
    if(percent_f < 0) {
        printf("[Warn ] Battery percentage below 0%%: %d%%\n", (uint8_t)percent_f);
        bs.percent = 0;
    }
    //bs.percent = 80;
    //printf("[Info ] Battery is at %.2f%%\n", percent_f);

<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
    printf("[Log ] VBAT: %4.0f mV\n", vbat_f);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    float neg = 1.0;

    // Read battery current draw
    // Convert to 2's compliment - negative means discharge
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c
    //pw_pmic_read_reg(REG_IBAT_ADC, buf, 2);
    //raw_val = REG_IBAT_ADC_VAL(buf16[0]);
    //if(raw_val > 0x1fff) { raw_val = (raw_val^0x3fff) + 1; neg = -1.0; }
    //float ibat_f = neg * (float)(raw_val) / (float)(0x3e8) * 4000.0;
    //printf("[Log ] IBAT: %4.0f mA\n", neg * (float)(raw_val) / (float)(0x3e8) * 4000.0);
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
=======
    pw_pmic_read_reg(REG_IBAT_ADC, buf, 2);
    raw_val = REG_IBAT_ADC_VAL(buf16[0]);
    if(raw_val > 0x1fff) { raw_val = (raw_val^0x3fff) + 1; neg = -1.0; }
    printf("[Log ] IBAT: %4.0f mA\n", neg * (float)(raw_val) / (float)(0x3e8) * 4000.0);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // Read bus current draw
    //pw_pmic_read_reg(REG_IBUS_ADC, buf, 2);
    //raw_val = REG_IBUS_ADC_VAL(buf16[0]);
    //if(raw_val > 0x3fff) { raw_val = (raw_val^0x7fff) + 1; neg = -1.0; }
    //printf("[Log ] IBUS: %4.0f mA\n", neg * (float)(raw_val) / (float)(0x7d0) * 4000.0);
    
    // Read system voltage
    // This gets converted down, but good to note what we're feeding the screen
    pw_pmic_read_reg(REG_VSYS_ADC, buf, 2);
    raw_val = REG_VSYS_ADC_VAL(buf16[0]);
<<<<<<< HEAD:src/drivers/battery/battery_pico_bq25628e.c
<<<<<<< HEAD
    float vsys_f = (float)raw_val * 5572.0 / (float)0x0af0;
    //printf("[Log ] VSYS: %4.0f mV\n", (float)(raw_val) / (float)(0xaf0) * 5572.0);

    printf("[Log  ] {\"VBAT_mV\": %4.0f, \"VSYS_mV\": %4.0f, \"Status\": \"%s\", \"ADC_conversion_ms\": %d}\n",
            vbat_f, vsys_f, CHARGE_STATUS_SHORT[charge_status], adc_conversion_time
    );
=======
    printf("[Log ] VSYS: %4.0f mV\n", (float)(raw_val) / (float)(0xaf0) * 5572.0);
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
=======
    float vsys_f = (float)raw_val * 5572.0 / (float)0x0af0;
    //printf("[Log ] VSYS: %4.0f mV\n", (float)(raw_val) / (float)(0xaf0) * 5572.0);

    int len = snprintf(log_staging, 128,
            "[Log  ] {\"VBAT_mV\": %4.0f, \"VSYS_mV\": %4.0f, \"Status\": \"%s\", \"Mode\": \"%s\", \"Timestamp\":%lu}\n",
            vbat_f, vsys_f, CHARGE_STATUS_SHORT[charge_status], (pw_power_get_mode())?"Sleep":"Normal", pw_time_get_rtc()
            );
    //printf("[Log  ] {\"VBAT_mV\": %4.0f, \"VSYS_mV\": %4.0f, \"Status\": \"%s\", \"ADC_conversion_ms\": %d}\n",
            //vbat_f, vsys_f, CHARGE_STATUS_SHORT[charge_status], adc_conversion_time
    //);
    printf(log_staging);
    pw_log(log_staging, len);
>>>>>>> 5cad753 (rebase survival):src/drivers/battery/bq25628e_rp2xxx_i2c.c

    // ADC gets auto-disabled if we're in one-shot mode


    return bs;
}
