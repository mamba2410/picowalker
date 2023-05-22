/// @file picowalker.c

#include <stdint.h>
#include <stdbool.h>

#include "picowalker.h"
#include "buttons.h"
#include "screen.h"
#include "states.h"
#include "rand.h"
#include "states.h"
#include "timer.h"
#include "globals.h"
#include "utils.h"
#include "ir/ir.h"
#include "eeprom.h"
#include "eeprom_map.h"
#include "accel.h"

/**
 *  Entry for the picowalker
 */
void walker_entry() {

    // Setup IR uart and rx interrupts
    pw_ir_init();
    pw_button_init();
    pw_screen_init();
    pw_eeprom_init();
    pw_accel_init();
    pw_srand(0x12345678);

    if(!pw_eeprom_check_for_nintendo()) {
        printf("'nintendo' not found, resetting\n");
        pw_eeprom_reset(true, true);
    }

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)&walker_info_cache,
        sizeof(walker_info_cache)
    );

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)&health_data_cache,
        sizeof(health_data_cache)
    );

    // swap BE in eeprom to LE in host
    health_data_cache.today_steps   = swap_bytes_u32(health_data_cache.today_steps);
    health_data_cache.total_steps   = swap_bytes_u32(health_data_cache.total_steps);
    health_data_cache.last_sync     = swap_bytes_u32(health_data_cache.last_sync);
    health_data_cache.total_days    = swap_bytes_u16(health_data_cache.total_days);
    health_data_cache.current_watts = swap_bytes_u16(health_data_cache.current_watts);

    if(walker_info_cache.flags & WALKER_INFO_FLAG_INIT) {
        pw_set_state(STATE_SPLASH);
    } else {
        pw_set_state(STATE_FIRST_CONNECT);
    }

    uint64_t now, prev_screen_redraw, prev_accel_check, td;
    prev_screen_redraw = pw_now_us();
    prev_accel_check = prev_screen_redraw;

    //health_data_cache.be_total_steps = swap_bytes_u32(99999);
    //health_data_cache.be_today_steps = swap_bytes_u32(99999);
    //health_data_cache.be_current_watts = swap_bytes_u16(9999);


    // Event loop
    // BEWARE: Could (WILL) receive interrupts during this time
    while(true) {
        // TODO: Things to do regardless of state (eg check steps, battery etc.)
        now = pw_now_us();
        td = (prev_accel_check>now)?(prev_accel_check-now):(now-prev_accel_check);
        if(td > ACCEL_NORMAL_SAMPLE_TIME_US) {
            prev_accel_check = now;
            pw_accel_process_steps();
        }

        // Run current state's event loop
        pw_state_run_event_loop();

        // Update screen since (presumably) we aren't doing anything time-critical
        now = pw_now_us();
        td = (prev_screen_redraw>now)?(prev_screen_redraw-now):(now-prev_screen_redraw);
        if(td > SCREEN_REDRAW_DELAY_US) {
            prev_screen_redraw = now;
            pw_state_draw_update();
        }

    }

}

