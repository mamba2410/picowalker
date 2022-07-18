// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>

#include "ir_comms.h"
#include "buttons.h"
#include "drivers/ssd1327.h"


void walker_entry();

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

int main() {
	bi_decl(bi_program_description("picowalker"));

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	stdio_init_all();

    printf("Hello, picowalker!");

	walker_entry();

}

void walker_entry() {

	// Setup IR uart and rx interrupts
	setup_ir_uart();
	setup_buttons();


	ssd1327_t oled = {
			i2c: i2c0,
			speed: 100*1000,
			sda: 4, // GP4
			scl: 5, // GP5
	};
	oled_init(oled);

	// init lcd
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
