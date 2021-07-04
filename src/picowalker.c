// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>


int main() {
	bi_decl(bi_program_description("picowalker"));

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
	stdio_init_all();

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	printf("Hello, World!\n");

	while(1) {
		gpio_put(LED_PIN, 1);
		sleep_ms(250);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);
	}
}
