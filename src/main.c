#include "pico/stdlib.h"
#include "RP2350TouchLCD128.h"


int main() {
    //stdio_init_all();
    RP2350TouchLCD128PW();

    while(1)
    {
        lv_timer_handler();
        sleep_ms(5);
    }

    return 0;
}