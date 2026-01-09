#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "board_resources.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/sync.h"
#include "hardware/xosc.h"
#include "hardware/powman.h"
#include "pico/sleep.h"
#include "pico/stdlib.h"
#include "stdio.h"

static volatile bool power_should_sleep;
extern volatile bool acknowledge_button_presses;


void pw_power_enter_light_sleep() {
    acknowledge_button_presses = false;

    // Clocks allowed to run:
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_REF_POWMAN_BITS;
    clocks_hw->sleep_en1 = CLOCKS_SLEEP_EN1_CLK_REF_TICKS_BITS | CLOCKS_SLEEP_EN1_CLK_SYS_TIMER0_BITS;
    scb_hw->scr |= ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);

    // START OF DANGER ZONE
    sleep_run_from_xosc();

    __wfi();

    sleep_power_up();
    // END OF DANGER ZONE

    //power_should_sleep = false;
    //set_user_idle_timer();

    acknowledge_button_presses = true;
}


void light_sleep_timer_callback() {
    hw_clear_bits(&timer_hw->intr, 1u<<LIGHT_SLEEP_ALARM_NUM);
}


void pw_power_light_sleep_for(uint32_t delay_ms) {
    // Start hardware timer
    hw_set_bits(&timer_hw->inte, 1u<<LIGHT_SLEEP_ALARM_NUM);
    uint user_idle_irq_num = timer_hardware_alarm_get_irq_num(timer_hw, LIGHT_SLEEP_ALARM_NUM);
    irq_set_exclusive_handler(user_idle_irq_num, light_sleep_timer_callback);
    irq_set_enabled(user_idle_irq_num, true);

    // Note: timer hardware is 32 bits in microseconds
    // So that's max 4294 seconds since start
    uint64_t target = timer_hw->timerawl + (1000*delay_ms);

    timer_hw->alarm[LIGHT_SLEEP_ALARM_NUM] = (uint32_t)target;

    // Go to sleep
    pw_power_enter_light_sleep();
}

