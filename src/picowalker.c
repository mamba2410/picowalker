// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>

#include "walker.h"
#include "pwroms.h"


int main() {
	bi_decl(bi_program_description("picowalker"));

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
	stdio_init_all();

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);


	size_t s = pw_walker_size();

	while(1) {
		printf("\nHello, World!\n");
		puts(eeprom);					// First bytes should be the string "nintendo\n"

		gpio_put(LED_PIN, 1);
		sleep_ms(1000);

		gpio_put(LED_PIN, 0);
		sleep_ms(1000);
	}
}


