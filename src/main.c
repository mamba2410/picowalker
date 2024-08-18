#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/clocks.h"
#include "hardware/pll.h"

#include "picowalker.h"
#include "picowalker-defs.h"

int main() {
    bi_decl(bi_program_description("picowalker"));

    stdio_init_all();

    printf("Hello, picowalker!\n");

    walker_entry();

    // unreachable
    while(1);
}

