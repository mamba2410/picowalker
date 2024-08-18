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

    uint16_t colours[] = {0xf800, 0x07e0, 0x101f, 0xffff};
    uint8_t i = 0;
    //pw_screen_init();

    //amoled_draw_block(0, 0, 368, 447, 0x0000);

    //walker_entry();

    //while(1) {

    //    //amoled_draw_block(100, 250, 96, 64, colours[i]);
    //    amoled_draw_block(0, 0, 96, 64, colours[i]);
    //    i = (i+1)%4;

    //    sleep_ms(1000);
    //}

    // unreachable
    while(1);
}

