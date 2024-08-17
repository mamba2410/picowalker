#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "picowalker.h"

int main() {
    bi_decl(bi_program_description("picowalker"));

    stdio_init_all();

    printf("Hello, picowalker!\n");

    walker_entry();

    // unreachable
    while(1);
}

