#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/xosc.h"
#include "hardware/powman.h"
#include "pico/sleep.h"
#include "pico/stdlib.h"
#include "stdio.h"

#include "board_resources.h"
#include "../../picowalker_structures.h"
#include "../accel/bma400_rp2xxx_spi.h"
#include "../battery/bq25628e_rp2xxx_i2c.h"
#include "../input/buttons_rp2xxx_gpio.h"
#include "dormant_rp2xxx.h"
#include "../interrupts/rp2xxx_gpio.h"

static volatile bool power_should_sleep;
volatile bool power_sleep_enabled = true;
volatile pw_wake_reason_t wake_reason;
extern int lposc_value;

extern void pw_button_init();

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


void pw_power_disable_sleep() {
    power_sleep_enabled = false;
}


void pw_power_enable_sleep() {
    power_sleep_enabled = true;
}


void pw_power_enter_sleep() {

    // Going to sleep, we don't want to respond to button presses
    // TODO: move this to core
    acknowledge_button_presses = false;

    wake_reason = 0;

    struct timespec ts;
    aon_timer_get_time(&ts);
    powman_timer_set_1khz_tick_source_lposc_with_hz(lposc_value);
    aon_timer_set_time(&ts);

    // Actually do the sleep
    //printf("[Debug] Sleeping MCU at 0x%08x s\n", (uint32_t)ts.tv_sec);

    // === Start of danger zone ===

    // Set XOSC as dormant clock source
    // Also reconfigures UART to run from XOSC
    sleep_run_from_lposc();
    //sleep_run_from_xosc();

    // Only let POWMAN clock run
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_REF_POWMAN_BITS;
    clocks_hw->sleep_en1 = 0;
    scb_hw->scr |= ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);

    //gpio_set_dormant_irq_enabled(ACCEL_INT_PIN, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_dormant_irq_enabled(BAT_INT_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_dormant_irq_enabled(BUTTON_MIDDLE_PIN, GPIO_IRQ_EDGE_FALL, true);
    // We should also be allowed to wake from AON timer

    // Go to sleep
    rosc_set_dormant();
    //xosc_dormant();

    // Interrupts happen here straight after waking up
    // Note: peripheral clocks aren't up and running yet so IO will likely be wonky

    sleep_power_up();
    printf("[Debug] MCU is awake, wake reason: 0x%02x\n", wake_reason);

    // === End of danger zone ===

    // Re-configure buttons since the config got clobbered from sleeping
    pw_button_init();

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

