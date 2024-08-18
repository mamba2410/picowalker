#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/clocks.h"
#include "hardware/pll.h"

#include "picowalker.h"
#include "picowalker-defs.h"

int main() {
    bi_decl(bi_program_description("picowalker"));

    // Set up system PLL to 15MHz
    /*
    pll_init(pll_sys, 1, 1500*MHZ, 10, 10);

    clock_configure(clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        15*MHZ,
        15*MHZ
    );
    */

    stdio_init_all();

    printf("Hello, picowalker!\n");

    //walker_entry();

    uint16_t colours[] = {0xf800, 0x07e0, 0x001f, 0x00ff};
    uint8_t brightness[] = {0x1f, 0x3f, 0x7f, 0xff};
    uint8_t i = 0;
    pw_screen_init();


    //amoled_draw_block(0, 0, 368, 447, 0x07e0);

    amoled_send_1wire(0x51, 1, (uint8_t[]){0x3f});

    while(1) {

        //amoled_draw_block(100, 250, 96, 64, colours[i]);
        amoled_draw_block(0, 0, 96, 64, colours[i]);
        i = (i+1)%4;

        amoled_send_1wire(0x51, 1, (brightness+i));
        sleep_ms(1000);
    }

    // unreachable
    while(1);
}

