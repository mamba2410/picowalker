#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include <stdio.h>

#include "board_resources.h"
#include "bq25628e_rp2xxx_i2c.h"
#include "../interrupts/rp2xxx_gpio.h"
#include "../../picowalker-defs.h"

//#define I2C_READ_MASK   (0x80)
#define I2C_READ_MASK   (0x00)
#define I2C_WRITE_MASK  (0x00)

#define VBAT_ABS_MINIMUM_MV 3500.0f
#define VBAT_ABS_MAXIMUM_MV 4225.0f
#define VBAT_SAFE_MINIMUM_MV 3600.0f

static char log_staging[128] = "";

static const char* CHARGE_STATUS_STRINGS[4] = {
    "not charging",
    "trickle, pre-charge or fast charge",
    "taper charge (CV mode)",
    "top-off timer active charge",
};

static const char* CHARGE_STATUS_SHORT[4] = {
    "D", "CC", "CV", "TOP",
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

static const uint8_t ADC_SHIFTS[] = { 1, 2, 2, 2, 1, 1, 0, 0 };

typedef struct bq25628e_info_s {
    uint8_t interrupt_flags[4];
    uint8_t interrupt_mask[4];
    uint8_t interrupt_status[4];
    uint32_t adc_timeout_stamp;
    uint32_t adc_finished_time;
    bool irq;
    bool adc_done;
    bool fault;
    bool started_charging;
    bool stopped_charging;
    bool plugged;
    bool unplugged;
} bq25628e_info_t;

static volatile bq25628e_info_t pmic_info = {};

static void bq25628e_read_reg(uint8_t reg, uint8_t *buf, size_t len) {
    if(buf == NULL) { return; }
    i2c_write_blocking(BAT_I2C_HW, BAT_I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(BAT_I2C_HW, BAT_I2C_ADDR, buf, len, false);
}

static void bq25628e_write_reg(uint8_t reg, uint8_t *buf, size_t len) {
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

static void print_flags_and_status(uint8_t flags[3], uint8_t status[3]) {

        printf("[Debug] ============\n");
        printf("[Debug] Interrupt flags: CHARGER_FLAGS_0: 0x%02x; CHARGER_FLAGS_1: 0x%02x; FAULT_FLAGS_0: 0x%02x\n", flags[0], flags[1], flags[2]);
        printf("[Debug]     CHARGER_FLAGS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(flags[0] & 1)
                printf("%s (%d) |", CHARGER_FLAG_0_STRINGS[i], i);
            flags[0] >>= 1;
        }

        printf("\n[Debug]     CHARGER_FLAGS_1: ");
        for(size_t i = 0; i < 8; i++) {
            if(flags[1] & 1)
                printf("%s |", CHARGER_FLAG_1_STRINGS[i]);
            flags[1] >>= 1;
        }

        printf("\n[Debug]     FAULT_FLAGS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(flags[2] & 1)
                printf("%s |", FAULT_FLAG_STRINGS[i]);
            flags[2] >>= 1;
        }
        printf("\n");

        // Read status registers
        printf("[Debug] Charger status 0: 0x%02x; Charger status 1: 0x%02x, FAULT status 0: 0x%02x\n", status[0], status[1], status[2]);
        printf("[Debug]     CHARGER_STATUS_0: ");
        for(size_t i = 0; i < 8; i++) {
            if(status[0] & 1)
                printf("%s |", CHARGER_STATUS_0_STRINGS[i]);
            status[0] >>= 1;
        }
        printf("\n[Debug]     CHARGER_STATUS_1.CHG_STAT: %s\n", CHARGE_STATUS_STRINGS[(status[1]&REG_CHARGER_STATUS_1_CHG_STAT_MSK)>>3]);
        printf("[Debug]     FAULT_STATUS_0.TS_STAT: %s\n", TS_STAT_STRINGS[status[2]&0x07]);
        printf("[Debug]     FAULT_STATUS_0:");

        status[2] >>= 3;
        for(size_t i = 3; i < 8; i++) {
            if(status[2] & 1)
                printf("%s |", FAULT_STATUS_0_STRINGS[i]);
            status[2] >>= 1;
        }
        printf("\n");
}


void bq25628e_print_fault_reason() {
    uint8_t fault_flags = pmic_info.interrupt_flags[2];
    printf("[Error] Fault flags: (0x%02x) ", fault_flags);
    for(size_t i = 0; i < 8; i++) {
        if(fault_flags & 1) {
            printf("%s |", FAULT_FLAG_STRINGS[i]);
        }
        fault_flags >>= 1;
    }
    printf("\n");

    uint8_t fault_mask = pmic_info.interrupt_mask[2];
    printf("[Error] Fault mask: (0x%02x) ", fault_mask);
    for(size_t i = 0; i < 8; i++) {
        if(fault_mask & 1) {
            printf("%s |", FAULT_FLAG_STRINGS[i]);
        }
        fault_mask >>= 1;
    }
    printf("\n");

    uint8_t fault_status = pmic_info.interrupt_status[2];
    printf("[Error] Fault status: (0x%02x) ", fault_status);
    for(size_t i = 0; i < 8; i++) {
        if(fault_status & 1)
            printf("%s |", FAULT_STATUS_0_STRINGS[i]);
        fault_status >>= 1;
    }
    printf("\n");
}


void debug_read_pmic() {
    uint16_t buf16[8];
    for(size_t i = 0; i < 8; i++)
        buf16[i] = 0;
    bq25628e_read_reg(REG_IBUS_ADC, (uint8_t*)buf16, 16);

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


bool bq25628e_valid_fault(uint8_t fault_flags, uint8_t fault_status) {
    // VBUS can fault when unplugging power source
    // If the flag is set, check against the status to see if it really faulted
    bool was_vbus_fault = pmic_info.interrupt_flags[2] & REG_FAULT_FLAG_0_VBUS_FAULT_FLAG;
    bool vbus_status_bad = pmic_info.interrupt_status[2] & REG_FAULT_STATUS_0_VBUS_FAULT_STAT;
    bool valid_vbus_fault = was_vbus_fault && vbus_status_bad;
    
    // For now, just compare against mask
    uint8_t fault_mask = pmic_info.interrupt_mask[2];
    return (fault_flags & fault_mask) != 0;
}

uint8_t bq25628e_get_charge_status();

/*
 * Interrupt handler from the PMIC
 * Read flags registers to determine what caused the interrupt and clear them.
 */
void bq25628e_service_irq() {

    // Read `FLAG` registers to clear interrupts
    bq25628e_read_reg(REG_CHARGER_FLAG_0, pmic_info.interrupt_flags, 3);
    bq25628e_read_reg(REG_CHARGER_STATUS_0, pmic_info.interrupt_status, 3);

    // Latch up, don't reset. Otherwise interrupts before we check the flag
    // will cause things to spin forever
    if(pmic_info.interrupt_flags[0] & REG_CHARGER_FLAG_0_ADC_DONE_FLAG) {
        pmic_info.adc_done = true;
        pmic_info.adc_finished_time = pw_time_get_ms();
    }

    if(pmic_info.interrupt_flags[1] & REG_CHARGER_FLAG_1_CHG_FLAG) {
        uint8_t charge_status = bq25628e_get_charge_status(pmic_info.interrupt_status);
        if(charge_status == CHARGE_STATUS_DISCHARGING) {
            pmic_info.stopped_charging = true;
        } else {
            pmic_info.started_charging = true;
        }
    }

    if(pmic_info.interrupt_flags[1] & REG_CHARGER_FLAG_1_VBUS_FLAG) {
        uint8_t vbus_status = (pmic_info.interrupt_status[1] >> 0) & 0x07;
        if(vbus_status == 0x04) {
            pmic_info.plugged = true;
        } else {
            pmic_info.unplugged = true;
        }
    }

    if(bq25628e_valid_fault(pmic_info.interrupt_flags[2], pmic_info.interrupt_status[2])) {
        //print_flags_and_status(interrupt_flags, interrupt_status);
        pmic_info.fault = true;
    }

}


/*
 * Minimal IRQ - i.e. don't read registers
 * RP2350 doesn't like reading registers in IRQ when waking up from DORMANT
 */
void bq25628e_irq_minimal() {
    pmic_info.irq = true;
}


void bq25628e_configure_interrupts() {
    uint8_t buf[3];

    // Charger 0
    buf[0] = 
        // Allow all
        0;

    // Charger 1
    buf[1] = 
        // Allow all
        0;

    // Fault
    buf[2] = 
        REG_FAULT_MASK_0_VBUS_FAULT_MASK | // VBUS can fault on unplug, ignore it
        0;

    bq25628e_write_reg(REG_CHARGER_MASK_0, buf, 3);
}


void bq25628e_disable_charge_pin() {
    gpio_put(BAT_CE_PIN, 1);
}


void bq25628e_enable_charge_pin() {
    gpio_put(BAT_CE_PIN, 0);
}


void bq25628e_software_reset() {
    uint8_t buf[4];
    bq25628e_read_reg(REG_CHARGER_CONTROL_1, buf, 1);
    buf[0] &= ~REG_CHARGER_CONTROL_1_REG_RST_MSK;
    buf[0] |= REG_CHARGER_CONTROL_1_REG_RST;
    bq25628e_write_reg(REG_CHARGER_CONTROL_1, buf, 1);
}


uint8_t bq25628e_read_part_info() {
    uint8_t part_info;
    bq25628e_read_reg(REG_PART_INFO, &part_info, 1);
    return part_info;
}

void bq25628e_disable_watchdog() {
    uint8_t buf[4];

    // Default is 50s, can set up to 200s
    bq25628e_read_reg(REG_CHARGER_CONTROL_0, buf, 1);
    buf[0] &= ~REG_CHARGER_CONTROL_0_WATCHDOG_MSK;
    buf[0] |= REG_CHARGER_CONTROL_0_WATCHDOG_DISABLED;
    bq25628e_write_reg(REG_CHARGER_CONTROL_0, buf, 1);
}


void bq25628e_configure_adc_targets() {
    uint8_t buf[4];

    // Allow: VBAT, VSYS
    buf[0] = 
        REG_ADC_FUNCTION_DISABLE_0_VPMID |
        REG_ADC_FUNCTION_DISABLE_0_TDIE |
        REG_ADC_FUNCTION_DISABLE_0_IBAT |
        REG_ADC_FUNCTION_DISABLE_0_IBUS |
        REG_ADC_FUNCTION_DISABLE_0_VBUS |
        REG_ADC_FUNCTION_DISABLE_0_TS |
        0;
    bq25628e_write_reg(REG_ADC_FUNCTION_DISABLE_0, buf, 1);
}


void bq25628e_configure_adc() {
    uint8_t buf[4];

    buf[0] = REG_ADC_CONTROL_ADC_AVG_DISABLED |
             REG_ADC_CONTROL_ADC_RATE_ONESHOT |
             //REG_ADC_CONTROL_ADC_SAMPLE_9_BIT | // default, 3.75 ms / target
             REG_ADC_CONTROL_ADC_SAMPLE_10_BIT | // 7.5 ms / target
             //REG_ADC_CONTROL_ADC_SAMPLE_11_BIT | // 15 ms / target
             //REG_ADC_CONTROL_ADC_SAMPLE_12_BIT | // 30 ms / target
             REG_ADC_CONTROL_ADC_AVG_INIT_NEW | // Always start average with new value
             REG_ADC_CONTROL_ADC_EN_DISABLED; // don't enable yet

    bq25628e_write_reg(REG_ADC_CONTROL, buf, 1);
}


void bq25628e_start_adc_measurement() {
    uint8_t buf[4];
    bq25628e_read_reg(REG_ADC_CONTROL, buf, 1);
    buf[0] &= ~REG_ADC_CONTROL_ADC_EN_MSK;
    buf[0] |= REG_ADC_CONTROL_ADC_EN_ENABLED;
    bq25628e_write_reg(REG_ADC_CONTROL, buf, 1);

    pmic_info.adc_timeout_stamp = pw_time_get_ms() + ADC_TIMEOUT_MS;
    pmic_info.adc_finished_time = 0;

}


uint8_t bq25628e_get_charge_status(uint8_t status_regs[3]) {
    uint8_t charge_status = (status_regs[1]>>3)&0x03;
    return charge_status;
}


/**
 */ 
uint32_t bq25628e_get_adc_conversion_time() {
    uint32_t conversion_time = 0;
    
    if(pmic_info.adc_done) {
        uint32_t adc_start_time = pmic_info.adc_timeout_stamp - ADC_TIMEOUT_MS;
        conversion_time = pmic_info.adc_finished_time - adc_start_time;
    } else {
        conversion_time = ADC_TIMEOUT_MS;
    }

    pmic_info.adc_done = false;
    pmic_info.adc_timeout_stamp = 0;
    pmic_info.adc_finished_time = 0;

    return conversion_time;
}


bool bq25628e_adc_timed_out() {
    // No ADC running, so we can't be timed out
    if(pmic_info.adc_timeout_stamp == 0) return false;
    if(pmic_info.adc_done) return false;

    uint32_t adc_start_time = pmic_info.adc_timeout_stamp - ADC_TIMEOUT_MS;
    uint32_t conversion_time = pw_time_get_ms() - adc_start_time;
    return conversion_time >= ADC_TIMEOUT_MS;
}


float bq25628e_read_vbat() {
    uint8_t buf[2];
    bq25628e_read_reg(REG_VBAT_ADC, buf, 2);
    uint16_t reg_val = ((uint16_t)buf[0] << 0) | ((uint16_t)buf[1] << 8);
    uint16_t adc_counts = REG_VBAT_ADC_VAL(reg_val);

    // Value here is 0x000 - 0xaf0, max resolution of 1.99mV
    float vbat = ((float)adc_counts * 5572.0 / (float)0x0af0);

    return vbat;
}


float bq25628e_read_vsys() {
    uint8_t buf[2];
    bq25628e_read_reg(REG_VSYS_ADC, buf, 2);
    uint16_t reg_val = ((uint16_t)buf[0] << 0) | ((uint16_t)buf[1] << 8);
    uint16_t adc_counts = REG_VSYS_ADC_VAL(reg_val);

    float vsys = (float)adc_counts * 5572.0 / (float)0x0af0;

    return vsys;
}


/**
 * ============================================================================
 * Picowalker core functionality
 * ============================================================================
 */
void pw_battery_shutdown() {
    uint8_t val = REG_CHARGER_CONTROL_2_BATFET_CTRL_SHUTDOWN;
    bq25628e_write_reg(REG_CHARGER_CONTROL_2, &val, 1);
}

void pw_battery_init() {

    board_i2c_init();

    gpio_init(BAT_CE_PIN);
    gpio_set_dir(BAT_CE_PIN, GPIO_OUT);

    // Disable charge for now
    bq25628e_disable_charge_pin();

    // Set up INT pin with callback
    gpio_init(BAT_INT_PIN);
    gpio_pull_up(BAT_INT_PIN);
    gpio_set_irq_enabled_with_callback(BAT_INT_PIN, GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);

    bq25628e_software_reset();

    uint8_t part_info = bq25628e_read_part_info();
    printf("[Info ] BQ25628E part info: 0x%02x\n", part_info);

    // Read `FLAG` registers to clear interrupts
    // (possibly) important to clear "power-on" interrupt
    uint8_t buf[4] = {};
    bq25628e_read_reg(REG_CHARGER_FLAG_0, buf, 3);

    // Disable the watchdog timer so that its always in host mode.
    bq25628e_disable_watchdog();
    bq25628e_configure_interrupts();
    bq25628e_configure_adc_targets();
    bq25628e_configure_adc();

    bq25628e_enable_charge_pin();

    // TODO: Set REG_CHARGER_CONTROL_3.IBAT_PK to 0x0 for 1.5A discharge limit
    // TODO: Charge current limit default 320mA, can reprogram. See REG_CHARGE_CURRENT_LIMIT
    // TODO: VSYSMIN is 3520mV, can change with REG_MINIMAL_SYSTEM_VOLTAGE

}


void pw_power_start_measurement() {

    // Don't start if one is already in progress
    bool adc_in_progress = pmic_info.adc_timeout_stamp != 0;
    if(adc_in_progress) {
        return;
    }

    bq25628e_start_adc_measurement();

}


/**
 * Return true if ADC finished, timed out or faulted
 * Don't clear since process_battery() needs the information too
 */
bool pw_power_result_available() {
    bool is_active = pmic_info.adc_timeout_stamp > 0;
    bool is_finished = pmic_info.adc_done || (pw_time_get_ms() >= pmic_info.adc_timeout_stamp);
    return is_active && is_finished;
}


/*
 * Callback function called periodically by picowalker-core
 * Runs in normal context
 */
pw_battery_status_t pw_power_get_status() {
    pw_battery_status_t bs = {.percent = 0, .flags = 0x00};

    // Minimal interrupt handler didn't read anything, so we do that now
    if(pmic_info.irq) {
        pmic_info.irq = false;
        bq25628e_service_irq();
    }


    // Immediately check faults
    if(pmic_info.fault) {
        pmic_info.fault = false;
        bq25628e_print_fault_reason();
        sleep_ms(100);
        bs.flags |= PW_BATTERY_STATUS_FLAGS_FAULT;
        // Faulted, so we leave now
        return bs;
    }

    // Not faulted, so we must have ADC measurement or some other non-critical interrupt

    if(pmic_info.started_charging) {
        pmic_info.started_charging = false;
        bs.flags |= PW_BATTERY_STATUS_FLAGS_CHARGING;
    }

    if(pmic_info.stopped_charging) {
        pmic_info.stopped_charging = false;
        bs.flags |= PW_BATTERY_STATUS_FLAGS_CHARGE_ENDED;
    }

    if(pmic_info.plugged) {
        pmic_info.plugged = false;
        bs.flags |= PW_BATTERY_STATUS_FLAGS_PLUGGED;
    }

    if(pmic_info.unplugged) {
        pmic_info.unplugged = false;
        bs.flags |= PW_BATTERY_STATUS_FLAGS_UNPLUGGED;
    }

    //if(!pw_power_result_available()) {
    if(bq25628e_adc_timed_out()) {
        //printf("[Warn ] Asking for battery measurement when one isn't available\n");
        printf("[Warn ] Battery ADC measurement exceeded %d ms\n", ADC_TIMEOUT_MS);
        bs.flags |= PW_BATTERY_STATUS_FLAGS_TIMEOUT;
        // clear ADC measurement
        uint32_t conversion_time = bq25628e_get_adc_conversion_time();
    }

    // Lastly, check if we have an ADC measurement. If not, we ditch early.
    if(!pw_power_result_available()) return bs;

    // We didn't leave, so we must have a measurement.
    uint32_t conversion_time = bq25628e_get_adc_conversion_time();
    bs.flags |= PW_BATTERY_STATUS_FLAGS_MEASUREMENT;

    // Read ADC targets
    float vbat_f = bq25628e_read_vbat();
    //float vsys_f = bq25628e_read_vsys();

    // Convert vbat to percentage
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

    uint8_t charge_status = bq25628e_get_charge_status(pmic_info.interrupt_status);

    // Log and print
    int len = snprintf(log_staging, sizeof(log_staging),
            "{\"vbat\":%4.0f,\"status\":\"%s\",\"mode\":\"%s\",\"time\":%lu}\n",
            vbat_f, CHARGE_STATUS_SHORT[charge_status], (pw_power_get_mode())?"S":"N", pw_time_get_rtc()
            );
    printf(log_staging);

    return bs;
}

