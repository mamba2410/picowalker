// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <pico/time.h>
#include <hardware/i2c.h>
#include <string.h>

#include "ir_comms.h"
#include "buttons.h"
#include "screen.h"
#include "states.h"

#include "trainer_info.h"

#define SCREEN_REDRAW_DELAY_US  1000000 // 1 second

void walker_entry();

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

int main() {
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
	bi_decl(bi_program_description("picowalker"));

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	stdio_init_all();

    printf("Hello, picowalker!\n");

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c / oled_i2d example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#endif

	walker_entry();

}

void walker_entry() {

    pw_check_struct_sizes();

	// Setup IR uart and rx interrupts
	setup_ir_uart();
	pw_setup_buttons();
    pw_screen_init();

    pw_set_state(STATE_SPLASH);

    absolute_time_t now, prev_screen_redraw;
    prev_screen_redraw = get_absolute_time();

	// Event loop
    // BEWARE: Could (WILL) receive interrupts during this time
	while(1) {
        // TODO: Things to do regardless of state (eg check steps, battery etc.)

        // Run current state's event loop
        pw_state_run_event_loop();

        // Update screen since (presumably) we aren't doing anything time-critical
        // TODO: Could break if we interrupt half way through, draw something else, and return
        now = get_absolute_time();
        if(absolute_time_diff_us(prev_screen_redraw, now) > SCREEN_REDRAW_DELAY_US) {
            prev_screen_redraw = now;
            pw_state_draw_update();
        }

	}

}



