#include "buttons_rp2350_cst816s.h"

touch_screen touch;

/********************************************************************************
Function: Resets Touch Functionality
Parameters:
********************************************************************************/
 void touch_reset()
 {
    gpio_put(TOUCH_PIN_RST, 0);
    sleep_ms(100);
    gpio_put(TOUCH_PIN_RST, 1);
    sleep_ms(100);
 }

/********************************************************************************
Function: 
Parameters:
********************************************************************************/
void touch_wake_up()
{
    gpio_put(TOUCH_PIN_RST, 0);
    sleep_ms(10);
    gpio_put(TOUCH_PIN_RST, 1);
    sleep_ms(50);

    uint8_t data[2] = {TOUCH_DISABLE_AUTO_SLEEP, 0x01};
    i2c_write_blocking(I2C, TOUCH_ADDRESS, data, 2, false);
}
