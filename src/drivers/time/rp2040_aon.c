#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
//#include "pico/stdlib.h"

#include "pico/bootrom.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/aon_timer.h"

#include "../../picowalker_structures.h"

#define UNIX_TIME_OFFSET 946684800ul
#define TIMER_INTERVAL_SEC 60

#define RTC_CLK_PIN 20

#define USE_EXTERNAL_RTC false

static pw_dhms_t last_check = {0,};
static struct timespec next_alarm = {0,};
uint16_t lposc_value = 32768;

extern pw_wake_reason_t wake_reason;

/*
 * ============================================================================
 * Functions required by driver
 * ============================================================================
 */

/********************************************************************************
 * @brief           Timer Callback
 * @param N/A
********************************************************************************/
void pw_timer_periodic_callback() 
{
    wake_reason |= PW_WAKE_REASON_RTC;
}

/********************************************************************************
 * @brief           Intialize RTC
 * @param sync_time is in seconds since 1st Jan 2000
********************************************************************************/
void pw_time_init_rtc(uint32_t sync_time) 
{
    // `sync_time` is in seconds since 1st Jan 2000
    // RP2040 - Use basic hardware timers, avoid AON functions that break USB
    lposc_value = 32768;
    printf("[Debug] RP2040 using basic timer fallback, sync_time: 0x%08x\n", sync_time);
    
    // Store the sync time in a simple variable instead of using AON timer
    static uint32_t rp2040_rtc_time = 0;
    rp2040_rtc_time = sync_time;
    
    // Don't use AON timer functions on RP2040 - they interfere with USB
    printf("[Debug] RP2040 RTC initialized without AON timer\n");
}

/********************************************************************************
 * @brief           Set RTC
 * @param sync_time is in seconds since 1st Jan 2000
********************************************************************************/
void pw_time_set_rtc(uint32_t sync_time) 
{
    // We don't need to run as unix time, its simpler to just use PW time
    // RP2040 - Store in simple variable, avoid AON timer
    static uint32_t rp2040_rtc_time = 0;
    rp2040_rtc_time = sync_time;
    printf("[Debug] RP2040 Setting RTC to 0x%08x (stored in variable)\n", sync_time);
}

/********************************************************************************
 * @brief           Get RTC
 * @param N/A
********************************************************************************/
uint32_t pw_time_get_rtc() 
{
    // RP2040 - Use hardware timer instead of AON timer
    static uint32_t rp2040_rtc_time = 0;
    uint64_t current_time_us = to_us_since_boot(get_absolute_time());
    uint32_t elapsed_seconds = (uint32_t)(current_time_us / 1000000ULL);
    printf("[Debug] RP2040 RTC time: %u seconds since boot\n", elapsed_seconds);
    return rp2040_rtc_time + elapsed_seconds;
}

/********************************************************************************
 * @brief           Get Days Hours Minutes Seconds
 * @param N/A
********************************************************************************/
pw_dhms_t pw_time_get_dhms() 
{
    // RP2040 - Use hardware timer instead of AON timer
    uint64_t current_time_us = to_us_since_boot(get_absolute_time());
    uint64_t units = current_time_us / 1000000ULL;
    pw_dhms_t dhms;
    dhms.seconds = units%60;
    units /= 60;
    dhms.minutes = units%60;
    units /= 60;
    dhms.hours = units%24;
    dhms.days = units/24;
    return dhms;
}

/********************************************************************************
 * @brief           Get Time in Microseconds
 * @param N/A
********************************************************************************/
uint32_t pw_time_get_us() 
{
    return (uint32_t)to_us_since_boot(get_absolute_time());
}

/********************************************************************************
 * @brief           Get Time in Milliseconds
 * @param N/A
********************************************************************************/
uint32_t pw_time_get_ms() 
{
    return (uint32_t)to_ms_since_boot(get_absolute_time());
}

/********************************************************************************
 * @brief           Delay in Milliseconds
 * @param ms        milliseconds
********************************************************************************/
void pw_time_delay_ms(uint32_t ms) 
{
    sleep_ms(ms);
}

/********************************************************************************
 * @brief           Delay in Microseconds
 * @param us        microseconds
********************************************************************************/
void pw_time_delay_us(uint32_t us) 
{
    sleep_us(us);
}


/*
 * ============================================================================
 * Old functions
 * ============================================================================
 */

 /********************************************************************************
 * @brief           
 * @param N/A
********************************************************************************/
uint64_t pw_now_us() 
{
    absolute_time_t now = get_absolute_time();
    // this is mega hacky, but I don't know how else to get an
    // absolute time stamp in us as a u64
    //return now._private_us_since_boot;
    return (uint64_t)now;
}

/********************************************************************************
 * @brief           
 * @param N/A
********************************************************************************/
void pw_timer_delay_ms(uint64_t ms) 
{
    sleep_ms(ms);
}

/********************************************************************************
 * @brief           
 * @param N/A
********************************************************************************/
void pw_ir_delay_ms(uint64_t ms) 
{
    pw_timer_delay_ms(ms);
}