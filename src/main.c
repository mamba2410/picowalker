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

    /*
    // Program the EEPROM over debug uart
    // Send a full rom file using `cat`
    // Beware: no validation is done
    pw_eeprom_init();

    uint16_t address = 0;
    uint8_t page[128];
    while(1){
        if(uart_is_readable(uart0)) {
            page[address&0x7f] = uart_getc(uart0);
            address++;
            if((address&0x7f) == 0) {
                pw_eeprom_write(address-128, page, 128);
                printf("Written page to 0x%04x\n", address-128);
            }
        }
    }
    */

    // Start picowalker-core
    walker_entry();

    // unreachable
    while(1);
}

