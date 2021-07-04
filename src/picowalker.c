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


void walker_entry() {
	// set stack
	// Zero BSS and copy data
	// Basic config
	// init accelerometer
	// if WDT_TCSRWD1&1 != 0 {
	// 	reset_caused by watchdog
	// 	get byte at 0x0072 in eeprom, increment then store again
	// } else {
	// 	reset_cause_analyse
	// 	zero 0x3E bytes at 0xF780 (some buffer)
	// }

	/*
	 *	Pseudocode program flow, trying to decode it from the disassembly
	 */

	/*
	 * Set stack
	 * Zero BSS and copy data
	 * Basic IO config (?)
	 * Init accelerometer
	 * Check WDT_TCSRWD1, if bit zero is set, reset caused by watchdog timer, inc byte at 0x0072
	 * If not, analyse cause of reset
	 * 		zero 0x3E bytes of RAM-cached health data
	 * 		(put number of written bytes in 0xF7A8 ?)
	 *
	 * 		Continue at 0x0306
	 *
	 *
	 *
	 */
}
