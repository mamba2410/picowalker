#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/xosc.h"
#include "pico/sleep.h"
#include "pico/stdlib.h"

#include "board_resources.h"
#include "../../picowalker-defs.h"
#include "../accel/bma400_rp2xxx_spi.h"
#include "../battery/bq25628e_rp2xxx_i2c.h"
#include "../input/buttons_rp2xxx_gpio.h"
#include "dormant_rp2xxx.h"

static volatile bool power_should_sleep;
volatile bool power_sleep_enabled = true;
pw_wake_reason_t wake_reason;
extern lposc_value;
extern lposc_value;

void user_idle_callback(void) {
    // Clear interrupt
    hw_clear_bits(&timer_hw->intr, 1u<<USER_IDLE_ALARM_NUM);
    if(power_sleep_enabled) {
        power_should_sleep = true;
    }
}

/*
 * Note that we need to keep "refreshing" the same timer rather than
 * adding new timers, otherwise the MCU would sleep after *every* button press
 * rather than just the last one.
 */
void set_user_idle_timer() {
    hw_set_bits(&timer_hw->inte, 1u<<USER_IDLE_ALARM_NUM);
    uint user_idle_irq_num = timer_hardware_alarm_get_irq_num(timer_hw, USER_IDLE_ALARM_NUM);
    irq_set_exclusive_handler(user_idle_irq_num, user_idle_callback);
    irq_set_enabled(user_idle_irq_num, true);

    // Note: timer hardware is 32 bits in microseconds
    // So that's max 4294 seconds since start
    uint64_t target = timer_hw->timerawl + (1000*USER_IDLE_TIMEOUT_MS);

    timer_hw->alarm[USER_IDLE_ALARM_NUM] = (uint32_t)target;
}

void pw_power_init() {
    pw_battery_init();

    set_user_idle_timer();
}

void pw_power_enter_sleep() {

    // Going to sleep, we don't want to respond to button presses
    // TODO: move this to core
    acknowledge_button_presses = false;

    wake_reason = 0;

    // Start the POWMAN timer from LPOSC which we aren't turning off
    struct timespec ts;
    aon_timer_get_time(&ts);
    printf("[Debug] Sleep saving time as 0x%08x s\n", (uint32_t)ts.tv_sec);

    powman_timer_set_1khz_tick_source_lposc_with_hz(lposc_value);
    //powman_timer_set_ms(powman_ms);
    aon_timer_set_time(&ts);

    // Actually do the sleep
    printf("[Info ] Sleeping MCU at 0x%08x s\n", (uint32_t)ts.tv_sec);

    // === Start of danger zone ===

    // Set XOSC as dormant clock source
    // Also reconfigures UART to run from XOSC
    sleep_run_from_lposc();
    //sleep_run_from_xosc();

    // Only let POWMAN clock run
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_REF_POWMAN_BITS;
    clocks_hw->sleep_en1 = 0;
    scb_hw->scr |= ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);
    sleep_run_from_lposc();
    //sleep_run_from_xosc();

    // Only let POWMAN clock run
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_REF_POWMAN_BITS;
    clocks_hw->sleep_en1 = 0;
    scb_hw->scr |= ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);

    gpio_set_dormant_irq_enabled(ACCEL_INT_PIN, IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS, true);
    gpio_set_dormant_irq_enabled(BAT_INT_PIN, IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS, true);
    gpio_set_dormant_irq_enabled(BUTTON_MIDDLE_PIN, IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS, true);
    //sleep_goto_dormant_until_pin(PIN_BUTTON_MIDDLE, true, false);
    // We should also be allowed to wake from AON timer
    rosc_set_dormant();
    //xosc_dormant();

    sleep_power_up();
    printf("[Info] MCU is awake\n");

    // === End of danger zone ===

    // Run POWMAN timer from XOSC since its more accurate
    //powman_ms = powman_timer_get_ms();
    //printf("[Debug] Waking powman timer with 0x%08x%08x ms\n", (uint32_t)(powman_ms>>32), (uint32_t)powman_ms);
    //powman_sec = powman_ms/1000;
    //printf("[Debug] Equivalent to 0x%08x s\n", powman_sec);
    //powman_timer_set_1khz_tick_source_xosc();
    //powman_timer_set_ms(powman_ms);

    aon_timer_get_time(&ts);
    printf("[Debug] Wake saving time as 0x%08x s\n", (uint32_t)ts.tv_sec);
    //powman_timer_set_1khz_tick_source_lposc_with_hz(lposc_value);
    powman_timer_set_1khz_tick_source_xosc();
    aon_timer_set_time(&ts);

    // TODO: Check what caused the wakeup, if it was AON timer then go back to sleep

    // TODO: Wait one second and sample pin again?
    // Can do that with another sleep timer clocked from AON

    //pw_accel_wake();
    //pw_flash_wake();
    //pw_eeprom_wake();

    //pw_ir_wake(); // Don't wake IR, we dont know if it was on

    // Re-configure buttons since the config got clobbered from sleeping
    pw_button_init();

    // Wake screen last for least weirdness
    //pw_screen_wake();

    // TODO: Change call when timer code gets updated
    power_should_sleep = false;
    set_user_idle_timer();

    // We're fully awake now, we can listen for buttons
    // TODO: move this to core
    acknowledge_button_presses = true;
}

bool pw_power_should_sleep() {
    return power_should_sleep;
}


pw_wake_reason_t pw_power_get_wake_reason() {
    return wake_reason;
}

void pw_power_clear_wake_reason(pw_wake_reason_t reason) {
    wake_reason &= ~reason;
}

