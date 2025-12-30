/// @file picowalker.c

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>

#include "accel.h"
#include "audio.h"
#include "buttons.h"
#include "eeprom.h"
#include "eeprom_map.h"
#include "globals.h"
#include "ir/ir.h"
#include "picowalker_structures.h"
#include "power.h"
#include "rand.h"
#include "states.h"
#include "screen.h"
#include "timer.h"
#include "utils.h"

struct {
    uint32_t now;
    uint32_t prev_screen_redraw;
    uint32_t prev_accel_check;
} walker_timings;

pw_state_t a1, a2;
pw_state_t *current_state = &a1, *pending_state = &a2;
screen_flags_t screen_flags;

void (*pw_current_loop)(void);

void pw_sleep_loop();
void pw_normal_loop();

void pw_setup() {
    // Setup IR uart and rx interrupts
    pw_power_init();
    pw_eeprom_init();
    pw_accel_init();
    pw_ir_init();
    pw_button_init();
    pw_screen_init();
    pw_audio_init();
    pw_srand(0x12345678);

    printf("[Info ] Peripherals initialised!\n");

    if(!pw_eeprom_check_for_nintendo()) {
        printf("[Info ] No \"nintendo\" found! Initialising EEPROM\n");
        pw_eeprom_reset(true, true);
    }

    int read_res;
    read_res = pw_eeprom_read_walker_info(&walker_info_cache);
    if(read_res < 0) {
        printf("[Warn ] Couldn't read walker info\n");
    }
    read_res = pw_eeprom_read_health_data(&health_data_cache);
    if(read_res < 0) {
        printf("[Warn ] Couldn't read health data\n");
    }

    pw_audio_volume = (health_data_cache.settings&SETTINGS_SOUND_MASK)>>SETTINGS_SOUND_OFFSET;
    pw_screen_set_brightness((health_data_cache.settings&SETTINGS_SHADE_MASK)>>SETTINGS_SHADE_OFFSET);

    if(walker_info_cache.flags & WALKER_INFO_FLAG_INIT) {
        current_state->sid = STATE_SPLASH;
        pending_state->sid = STATE_SPLASH;
    } else {
        current_state->sid = STATE_FIRST_COMMS;
        pending_state->sid = STATE_FIRST_COMMS;
    }

    pw_time_init_rtc(health_data_cache.last_sync);

    walker_timings.now = pw_time_get_us();
    walker_timings.prev_accel_check = 0;

    // Initialise the first states
    pw_screen_clear();
    STATE_FUNCS[current_state->sid].init(current_state, &screen_flags);
    STATE_FUNCS[current_state->sid].draw_init(current_state, &screen_flags);

    pw_current_loop = pw_normal_loop;
    printf("[Info ] Setup done, starting loop\n");
}


bool is_in_time_sensitive_state(pw_state_id_t state) {
    return (state == STATE_COMMS || state == STATE_FIRST_COMMS);
}

void pw_normal_loop() {
    uint64_t td;

    // Skip accel/battery checks if we can't afford to hang around
    /*
    if(!is_in_time_sensitive_state(current_state->sid)) {
        walker_timings.now = pw_time_get_us();
        td = (walker_timings.prev_accel_check>walker_timings.now)?(walker_timings.prev_accel_check-walker_timings.now):(walker_timings.now-walker_timings.prev_accel_check);
        if(td > ACCEL_NORMAL_SAMPLE_TIME_US) {
            walker_timings.prev_accel_check = walker_timings.now;
            pw_accel_process_steps();

            (void)pw_power_process_battery();
        }
    }
    */

    // Update power management
    pw_power_update();

    // Run current state's event loop
    STATE_FUNCS[current_state->sid].loop(current_state, pending_state, &screen_flags);

    // TODO: invalid sid checking
    if(pending_state->sid != current_state->sid) {
        STATE_FUNCS[current_state->sid].deinit(current_state, &screen_flags);

        pw_state_t *tmp = current_state;
        current_state = pending_state;
        pending_state = tmp;

        *pending_state = (pw_state_t) {
            0
        }; // clang-format why
        pending_state->sid = current_state->sid;

        pw_screen_clear();
        STATE_FUNCS[current_state->sid].init(current_state, &screen_flags);
        STATE_FUNCS[current_state->sid].draw_init(current_state, &screen_flags);
    }

    // Update screen since (presumably) we aren't doing anything time-critical
    walker_timings.now = pw_time_get_us();
    td = (walker_timings.prev_screen_redraw>walker_timings.now)?(walker_timings.prev_screen_redraw-walker_timings.now):(walker_timings.now-walker_timings.prev_screen_redraw);

    if(td > SCREEN_REDRAW_DELAY_US || PW_GET_REQUEST(current_state->requests, PW_REQUEST_REDRAW)) {
        walker_timings.prev_screen_redraw = walker_timings.now;
        STATE_FUNCS[current_state->sid].draw_update(current_state, &screen_flags);
        screen_flags.frame = (screen_flags.frame+1)%4;
        PW_CLR_REQUEST(current_state->requests, PW_REQUEST_REDRAW);

        // Draw low battery 
        if(power_context.show_battery_low_icon) {
            if((screen_flags.frame/2) == 0) {
                pw_screen_draw_from_eeprom(
                    0, 0,
                    8, 8,
                    PW_EEPROM_ADDR_IMG_LOW_BATTERY,
                    PW_EEPROM_SIZE_IMG_LOW_BATTERY
                );
            } else {
                pw_screen_clear_area(0, 0, 8, 8);
            }
        }
    }

    // TODO: Make this a dedicated RTC poll rather than relying on the sleep infrastructure
    pw_wake_reason_t wake_reason = pw_power_get_wake_reason();
    if(wake_reason & PW_WAKE_REASON_RTC) {
        pw_power_clear_wake_reason(PW_WAKE_REASON_RTC);
        pw_rtc_regular_processing();
        pw_accel_process_steps();
    }

    // Check if we should sleep
    if(pw_power_should_sleep()) {
        printf("[Debug] Sleep timeout hit, entering sleep\n");
        pw_current_loop = pw_sleep_loop;

        // Put peripherals to sleep
        pw_screen_sleep();
        pw_ir_sleep();

        // Pass control to "driver" and enter sleep
        // Driver should bring all clocks, hardware etc back to how it was left
        pw_power_enter_sleep();

        // Re-draw the screen
        //STATE_FUNCS[current_state->sid].draw_init(current_state, &screen_flags);

        // We are very unlikely to return
    }
}


/**
 * Loop to be run when we are in sleep mode
 * Start as if we just woke up because in some scenarios (rp2350) this is what happens
 */
void pw_sleep_loop() {

    pw_wake_reason_t wake_reason = pw_power_get_wake_reason();

    if(wake_reason & PW_WAKE_REASON_BATTERY) {
        //printf("[Debug] Wake because battery\n");
        //pw_power_start_measurement();
        //power_context.last_bat_check = pw_now_us();
        //while(!pw_power_result_available());
        pw_power_update();
        //uint8_t battery_level = pw_power_process_battery();
    }


    if(wake_reason & PW_WAKE_REASON_RTC) {
        //printf("[Debug] Wake because RTC\n");
        pw_rtc_regular_processing();
        pw_power_update();
        pw_power_start_measurement();
        //power_context.last_bat_check = pw_now_us();
        power_context.last_bat_check = pw_time_get_us();
        // Wait until its finished
        // TODO: Figure out how to go to sleep until its done
        //while(!pw_power_result_available());
        //uint8_t battery_level = pw_power_process_battery();
        //printf("[Debug] RTC wake checked battery: %d%%\n", battery_level);
    }

    if(wake_reason & PW_WAKE_REASON_ACCEL) {
        //printf("[Debug] Wake because accel\n");
        pw_accel_process_steps();
    }

    if(wake_reason & PW_WAKE_REASON_BUTTON) {
        //printf("[Debug] Wake because button\n");
        pw_accel_process_steps();
        pw_screen_wake();
        pw_screen_set_brightness((health_data_cache.settings&SETTINGS_SHADE_MASK)>>SETTINGS_SHADE_OFFSET);
        pw_screen_clear();

        if(current_state->sid == STATE_COMMS || current_state->sid == STATE_FIRST_COMMS) {
            pw_ir_wake();
        }

        // Re-draw the screen
        STATE_FUNCS[current_state->sid].draw_init(current_state, &screen_flags);
        pw_current_loop = pw_normal_loop;
        return;
    }

    // If nothing else to do, we go back to sleep
    pw_power_enter_sleep();
}

void pw_state_handle_input(pw_buttons_t b) {
    STATE_FUNCS[current_state->sid].input(current_state, &screen_flags, b);
}

void pw_ir_loop() {
}


/**
 *  Entry for the picowalker
 */
void pw_run() {

    pw_setup();

    // Event loop
    // BEWARE: Could (WILL) receive interrupts during this time
    while(true) {
        //pw_normal_loop();
        pw_current_loop();
    }

}

