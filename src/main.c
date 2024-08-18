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

    while(1) {
        pw_screen_init();
        sleep_ms(1000);
    }

    // unreachable
    while(1);
}

