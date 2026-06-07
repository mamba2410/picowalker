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

#include "dormant_rp2xxx.h"

static volatile bool power_should_sleep;
extern volatile bool acknowledge_button_presses;

extern void run_powman_timer_from_lposc();

void pw_power_enter_light_sleep() {
    acknowledge_button_presses = false;

    wake_reason = 0;

    // Clocks allowed to run:
    uint32_t clocks0 = clocks_hw->sleep_en0;
    uint32_t clocks1 = clocks_hw->sleep_en1;
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_REF_POWMAN_BITS;
    clocks_hw->sleep_en1 = CLOCKS_SLEEP_EN1_CLK_REF_TICKS_BITS | CLOCKS_SLEEP_EN1_CLK_SYS_TIMER0_BITS;
    scb_hw->scr |= ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);

    // START OF DANGER ZONE
    sleep_run_from_xosc();

    __wfi();

    sleep_power_up();
    run_powman_timer_from_lposc();
    // END OF DANGER ZONE

    // Restore clocks so calling `wfi` in a different place behaves as expected
    clocks_hw->sleep_en0 = clocks0;
    clocks_hw->sleep_en1 = clocks1;

    //power_should_sleep = false;
    //set_user_idle_timer();

    acknowledge_button_presses = true;
}


int64_t light_sleep_timer_callback(alarm_id_t id, void *data) {
    (void)id;
    (void)data;
    return 0;
}


void pw_power_light_sleep_for(uint32_t delay_ms) {
    // Start hardware timer
    add_alarm_in_ms(delay_ms, light_sleep_timer_callback, NULL, false);

    // Go to sleep
    pw_power_enter_light_sleep();
}

