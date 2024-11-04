#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/clocks.h"
#include "hardware/pll.h"

#include "drivers/eeprom_pico_m95512.h"
#include "picowalker.h"
#include "picowalker-defs.h"

int main() {
    bi_decl(bi_program_description("picowalker"));

    // Downclock the system to save a bit of power and allow the HSTX
    // to have a lower clock
    //set_sys_clock_khz(48000, true);

    stdio_init_all();

    printf("Hello, picowalker!\n");

    // Start picowalker-core
    walker_entry();

    // unreachable
    while(1);
}

