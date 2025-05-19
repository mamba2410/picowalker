#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//#include "pico/stdlib.h"

#include "hardware/powman.h"
#include "pico/bootrom.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/aon_timer.h"


#include "power_pico.h"
#include "../picowalker-defs.h"

#define UNIX_TIME_OFFSET 946684800ul
#define TIMER_INTERVAL_SEC 60

#define RTC_CLK_PIN 20

static pw_dhms_t last_check = {0,};
static struct timespec next_alarm = {0,};
uint16_t lposc_value = 32768;

/*
 * ============================================================================
 * Functions required by driver
 * ============================================================================
 */

void pw_timer_periodic_callback() {
    next_alarm.tv_sec += TIMER_INTERVAL_SEC;
    aon_timer_enable_alarm(&next_alarm, pw_timer_periodic_callback, true);
    wake_reason |= PW_WAKE_REASON_RTC;
}

void pw_time_init_rtc(uint32_t sync_time) {
    // `sync_time` is in seconds since 1st Jan 2000

    // Read LPOSC measured frequency and set LPOSC frequency
    otp_cmd_t cmd;
    cmd.flags = 0x11;
    uint8_t raw_value[4];
    int ret = rom_func_otp_access(raw_value, 4, cmd);
    if(ret) {
        printf("[Error] Couldn't read LPOSC value from OTP: %d\n", ret);
    } else {
        lposc_value = *(uint32_t*)raw_value;
        printf("[Debug] LPOSC factory measured at %d Hz\n", lposc_value);
    }

    // Set up powman timer
    powman_timer_stop();
    gpio_init(RTC_CLK_PIN);
    powman_hw->ext_time_ref = POWMAN_PASSWORD_BITS | (1<<4) | (0x01 << 0);
    lposc_value = 32768;
    powman_timer_set_1khz_tick_source_lposc_with_hz(lposc_value);
    //powman_timer_set_1khz_tick_source_xosc();
    printf("[Debug] Changed RTC to external lposc at %d Hz\n", lposc_value);

    // Convert pw time to unix time
    struct timespec ts = {0, 0};
    //ts.tv_sec = (uint64_t)(sync_time) + UNIX_TIME_OFFSET;
    
    // We don't need to run as unix time, its simpler to just use PW time
    //struct timespec ts = {0, 0};
    //ts.tv_sec = (uint64_t)(sync_time);
    ts.tv_sec |= sync_time;
    printf("[Debug] Initialising RTC to 0x%08x\n", sync_time);

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
    printf("[Debug] Setting RTC to 0x%08x\n", sync_time);
    aon_timer_set_time(&ts);
}

uint32_t pw_time_get_rtc() {
    uint64_t ms = powman_timer_get_ms();
    printf("[Debug] Powman timer has 0x%08x ms\n", (uint32_t)ms);
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

