// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <pico/time.h>
#include <hardware/gpio.h>

#include "ir_comms.h"


#define BUTTON_LEFT 20
#define BUTTON_MIDDLE 19
#define BUTTON_RIGHT 18

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

int main() {
	bi_decl(bi_program_description("picowalker"));

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	stdio_init_all();

	walker_entry();

}

void pw_irq_callback(uint gp, uint32_t events) {
	switch(gp) {
		case BUTTON_LEFT: printf("left\n"); break;
		case BUTTON_MIDDLE: printf("middle\n"); break;
		case BUTTON_RIGHT: printf("right\n"); break;
		default: break;
	}

}

void setup_buttons() {
	gpio_set_irq_enabled_with_callback(BUTTON_LEFT, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
	gpio_set_irq_enabled_with_callback(BUTTON_MIDDLE, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
	gpio_set_irq_enabled_with_callback(BUTTON_RIGHT, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
}


void walker_entry() {

	// Setup IR uart and rx interrupts
	setup_ir_uart();
	setup_buttons();

	// init lcd
	// init accel
	

	// For debug testing
	uint32_t time_ms;
	char buf[24];

	// Event loop
	while(1) {
		// Check steps
		// Check buttons
		// Switch state

		time_ms = to_ms_since_boot(get_absolute_time());
		gpio_put(LED_PIN, 1);
		sleep_ms(250);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);
		sprintf(buf, "Hello, world %d\n");
		printf(buf);

	}

}
