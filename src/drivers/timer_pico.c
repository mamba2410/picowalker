#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//#include "pico/stdlib.h"

#include "pico/time.h"
#include "pico/types.h"
#include "pico/aon_timer.h"

#include "power_pico.h"
#include "../picowalker-defs.h"

#define UNIX_TIME_OFFSET 946684800ul

static struct rtc_counters_s {
    uint8_t minutes;
    uint8_t hours;
    uint16_t days;
} rtc_counters;


void pw_rtc_callback() {
    // Should be called every minute by the AON timer
    rtc_counters.minutes += 1;
    
    // Set the reason for waking up, does nothing if we are already awake
    wake_reason |= PW_WAKE_REASON_RTC;

    // Schedule another RTC alarm
}

/*
 * ============================================================================
 * Functions required by driver
 * ============================================================================
 */
void pw_time_init_rtc(uint32_t sync_time) {
    // `sync_time` is in seconds since 1st Jan 2000

    // Convert pw time to unix time
    //struct timespec ts = {0, 0};
    //ts.tv_sec = (uint64_t)(sync_time) + UNIX_TIME_OFFSET;
    
    // We don't need to run as unix time, its simpler to just use PW time
    struct timespec ts = {0, 0};
    ts.tv_sec = (uint64_t)(sync_time);

    aon_timer_start(&ts);

}

void pw_time_set_rtc(uint32_t sync_time) {
    // We don't need to run as unix time, its simpler to just use PW time
    struct timespec ts = {0, 0};
    ts.tv_sec = (uint64_t)(sync_time);
    aon_timer_set_time(&ts);
}

/*
 * Returns flags of what periodic events need to be handled
 */
uint8_t pw_time_check_events() {
    uint8_t events = 0;

    if(rtc_counters.minutes >= 60) {
        rtc_counters.minutes -= 60;
        rtc_counters.hours += 1;
        events |= PW_EVENT_EVERY_HOUR;
    }

    if(rtc_counters.hours >= 24) {
        rtc_counters.hours -= 24;
        rtc_counters.days += 1;
        events |= PW_EVENT_EVERY_DAY;
    }
    
    // TODO: Check for minutes
    // TODO: Check for hours
    // TODO: Check for days

    return events;
}

uint64_t pw_time_now_us() {
    absolute_time_t now = get_absolute_time();
    return (uint64_t)now;
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

