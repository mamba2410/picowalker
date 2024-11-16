#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/timer.h"
#include "pico/sleep.h"
#include "pico/stdlib.h"

#include "../picowalker-defs.h"
#include "battery_pico_bq25628e.h"
#include "buttons_pico.h"
#include "power_pico.h"

static volatile bool power_should_sleep;

void user_idle_callback(void) {
    // Clear interrupt
    hw_clear_bits(&timer_hw->intr, 1u<<USER_IDLE_ALARM_NUM);
    power_should_sleep = true;
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

    // Turn off the peripherals
    printf("[Info] Sleeping peripherals\n");

    // Going to sleep, we don't want to respond to button presses
    acknowledge_button_presses = false;

    // Sleep the screen first so it doesn't do anything weird
    pw_screen_sleep();

    // Sleep the IR in case we were in the comms context
    pw_ir_sleep();

    pw_eeprom_sleep();
    //pw_flash_sleep();
    //pw_accel_sleep(); // Don't sleep accel, it stops counting steps


    // Actually do the sleep
    printf("[Info] Sleeping MCU\n");

    // === Start of danger zone ===

    // Set XOSC as dormant clock source
    // Also reconfigures UART to run from XOSC
    sleep_run_from_xosc();

    sleep_goto_dormant_until_pin(PIN_BUTTON_MIDDLE, true, false);

    // TODO: Wait one second and sample pin again?
    // Can do that with another sleep timer clocked from AON

    sleep_power_up();
    printf("[Info] MCU is awake\n");

    // === End of danger zone ===

    //pw_accel_wake();
    //pw_flash_wake();
    pw_eeprom_wake();

    //pw_ir_wake(); // Don't wake IR, we dont know if it was on

    // Re-configure buttons since the config got clobbered from sleeping
    pw_button_init();

    // Wake screen last for least weirdness
    pw_screen_wake();

    // TODO: Change call when timer code gets updated
    power_should_sleep = false;
    set_user_idle_timer();

    // We're fully awake now, we can listen for buttons
    acknowledge_button_presses = true;
}

bool pw_power_should_sleep() {
    return power_should_sleep;
}


