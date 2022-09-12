// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/i2c.h>
#include <string.h>

#include "ir_comms.h"
#include "buttons.h"
#include "screen.h"
#include "pw_images.h"
#include "states.h"


void walker_entry();
void oled_test();

const uint LED_PIN = PICO_DEFAULT_LED_PIN;


int main() {
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
	bi_decl(bi_program_description("picowalker"));

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	stdio_init_all();

    printf("Hello, picowalker!");

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c / oled_i2d example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#endif

	walker_entry();

}

void walker_entry() {

	// Setup IR uart and rx interrupts
	setup_ir_uart();
	setup_buttons();

    pw_screen_init();

    pw_set_state(STATE_SPLASH);

	// init accel

	// Event loop
	while(1) {
		// Check steps
		// Switch state

		gpio_put(LED_PIN, 1);
		sleep_ms(250);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);

	}

}



