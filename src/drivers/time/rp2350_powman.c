#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "hardware/powman.h"
#include "pico/bootrom.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/aon_timer.h"
#include "hardware/gpio.h"
#include "stdio.h"

#include "../sleep/dormant_rp2xxx.h"
#include "../../picowalker_structures.h"

#define UNIX_TIME_OFFSET 946684800ul
#define TIMER_INTERVAL_SEC 60

#define DEFAULT_LPOSC_FREQ 32768ul

static struct timespec next_alarm = {0,};
static uint32_t pw_lposc_freq = DEFAULT_LPOSC_FREQ;


void pw_timer_periodic_callback() {
    next_alarm.tv_sec += TIMER_INTERVAL_SEC;
    aon_timer_enable_alarm(&next_alarm, pw_timer_periodic_callback, true);
    wake_reason |= PW_WAKE_REASON_RTC;
}


void run_powman_timer_from_lposc() {
    // Requires sdk 2.1.2 since its bugged before that
    powman_timer_set_1khz_tick_source_lposc_with_hz(pw_lposc_freq);
}

static uint32_t read_lposc_value_from_otp() {
    // Read LPOSC measured frequency and set LPOSC frequency
    otp_cmd_t cmd;
    cmd.flags = 0x11;
    uint8_t raw_value[4];
    int ret = rom_func_otp_access(raw_value, 4, cmd);
    uint32_t val = 0;
    if(ret == BOOTROM_OK) {
        val = *(uint32_t*)raw_value;
        val &= (1<<16)-1; // Get the lower 16 bits of the value
    } else {
        printf("[Warn ] Error reading LPOSC value from bootrom\n");
    }
    return val;
}


static uint32_t setup_powman_timer_from_lposc() {
    uint32_t freq = read_lposc_value_from_otp();

    if(freq == 0) {
        // Couldn't read, so we use default frequency
        freq = DEFAULT_LPOSC_FREQ;
    }

    // Set powman timer to use LPOSC
    powman_hw->ext_time_ref = POWMAN_PASSWORD_BITS | 0;

    printf("[Info ] RTC using internal LPOSC at %lu Hz\n", freq);

    return freq;
}


static uint32_t setup_powman_timer_from_external_pin(int pin) {
    gpio_init(pin);
    uint32_t reg_val = 0;
    switch(pin) {
        case 12: reg_val = (1<<4) | 0x00; break;
        case 20: reg_val = (1<<4) | 0x01; break;
        case 14: reg_val = (1<<4) | 0x02; break;
        case 22: reg_val = (1<<4) | 0x03; break;
        default: {
            reg_val = 0x0;
            printf("[Warn ] External clock on pin %d not allowed, reverting to internal LPOSC\n", pin);
            return setup_powman_timer_from_lposc();
        }
    }

    // Set powman timer to use clock on external pin
    powman_hw->ext_time_ref = POWMAN_PASSWORD_BITS | reg_val;

    printf("[Info] RTC using external osc on pin %d at %lu Hz\n", pin, DEFAULT_LPOSC_FREQ);

    // Assume frequency is 
    return DEFAULT_LPOSC_FREQ;
}

/*
 * ============================================================================
 * Functions required by driver
 * ============================================================================
 */



void pw_time_init_rtc(uint32_t sync_time) {
    // `sync_time` is in seconds since 1st Jan 2000

    powman_timer_stop();

#if USE_EXTERNAL_RTC
    pw_lposc_freq = setup_powman_timer_from_external_pin(RTC_CLK_PIN);
#else
    pw_lposc_freq = setup_powman_timer_from_lposc();
#endif

    run_powman_timer_from_lposc();

    // Convert pw time to unix time
    struct timespec ts = {0, 0};
    //ts.tv_sec = (uint64_t)(sync_time) + UNIX_TIME_OFFSET;
    
    // We don't need to run as unix time, its simpler to just use PW time
    //struct timespec ts = {0, 0};
    //ts.tv_sec = (uint64_t)(sync_time);
    ts.tv_sec |= sync_time;
    printf("[Debug] Initialising RTC to 0x%08lx\n", sync_time);

    bool ok = aon_timer_set_time(&ts);
    if(ok) {
        powman_timer_set_ms(timespec_to_ms(&ts));
        powman_timer_start();
    }

    ts.tv_sec += TIMER_INTERVAL_SEC;
    next_alarm = ts;
    aon_timer_enable_alarm(&next_alarm, pw_timer_periodic_callback, true);
    
}

void pw_time_set_rtc(uint32_t sync_time) {
    // We don't need to run as unix time, its simpler to just use PW time
    struct timespec ts = {0, 0};
    //ts.tv_sec = (uint64_t)(sync_time);
    ts.tv_sec |= sync_time;
    //ts.tv_sec = (uint64_t)(sync_time) + UNIX_TIME_OFFSET;
    printf("[Debug] Setting RTC to 0x%08lx\n", sync_time);
    aon_timer_set_time(&ts);
}

uint32_t pw_time_get_rtc() {
    uint64_t ms = powman_timer_get_ms();
    //printf("[Debug] Powman timer has 0x%08x ms\n", (uint32_t)ms);
    return (uint32_t)(ms/1000);
}

pw_dhms_t pw_time_get_dhms() {
    uint64_t ms = powman_timer_get_ms();
    uint64_t units = ms/1000;
    pw_dhms_t dhms;
    dhms.seconds = units%60;
    units /= 60;
    dhms.minutes = units%60;
    units /= 60;
    dhms.hours = units%24;
    dhms.days = units/24;
    return dhms;
}

uint32_t pw_time_get_us() {
    absolute_time_t now = get_absolute_time();
    return (uint32_t)now;
}

uint32_t pw_time_get_ms() {
    absolute_time_t now = get_absolute_time();
    uint64_t long_now = (uint64_t)now;
    return (uint32_t)(long_now/1000);
}

void pw_time_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

void pw_time_delay_us(uint32_t us) {
    sleep_us(us);
}


/*
 * ============================================================================
 * Old functions
 * ============================================================================
 */

uint64_t pw_now_us() {
    absolute_time_t now = get_absolute_time();
    // this is mega hacky, but I don't know how else to get an
    // absolute time stamp in us as a u64
    //return now._private_us_since_boot;
    return (uint64_t)now;
}

void pw_timer_delay_ms(uint64_t ms) {
    sleep_ms(ms);
}

void pw_ir_delay_ms(uint64_t ms) {
    pw_timer_delay_ms(ms);
}

