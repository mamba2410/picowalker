// picowalker.c

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>

#include "walker.h"
#include "pwroms.h"
#include "allocators.h"


int main() {
	bi_decl(bi_program_description("picowalker"));

	stdio_init_all();

	emu_run();

}


void emu_run() {
	pw_walker_t walker;

	const struct pw_allocator alloc_sched_event = {
		pico_pw_alloc, pico_pw_realloc, NULL
	};

	const struct pw_allocator alloc_ram = {
		pico_pw_alloc, pico_pw_realloc, NULL
	};

	pw_walker_init( walker, flashrom, eeprom, &alloc_sched_event, &alloc_ram);
	pw_walker_sched_set_interactive(walker, false);
	pw_walker_reset(walker);

	pw_walker_run(walker, 1000);

}


