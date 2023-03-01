// picowalker.c

#include <stdint.h>
#include <stdbool.h>

#include "picowalker.h"
#include "buttons.h"
#include "screen.h"
#include "states.h"
#include "rand.h"
#include "states.h"
#include "timer.h"
#include "ir/pw_ir.h"

void walker_entry() {

	// Setup IR uart and rx interrupts
    pw_ir_init();
	pw_button_init();
    pw_screen_init();
    pw_eeprom_init();
    pw_srand(0x12345678);

    pw_set_state(STATE_SPLASH);

    uint64_t now, prev_screen_redraw, td;
    prev_screen_redraw = pw_now_us();

	// Event loop
    // BEWARE: Could (WILL) receive interrupts during this time
	while(true) {
        // TODO: Things to do regardless of state (eg check steps, battery etc.)

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

