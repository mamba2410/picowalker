#include "buttons_rp2350_cst816s.h"

touch_screen TOUCH;

/********************************************************************************
Function: Writes to I2C
Parameters:
        register_address : register address to write to
        value : value to write
********************************************************************************/
void i2c_write(uint8_t register_address, uint8_t value)
{
    uint8_t buffer[2] = {id, register_address};
    i2c_write_blocking(I2C, TOUCH_ADDRESS, buffer, 2, false);
}

/********************************************************************************
Function: Reads from I2C
Parameters:
        register_address : register address to read from
********************************************************************************/
void i2c_read(uint8_t register_address)
{
    uint8_t buffer;
    i2c_write_blocking(I2C, TOUCH_ADDRESS, &register_address, 1, true);
    i2c_read_blocking(I2C, TOUCH_ADDRESS, &buffer, 1, false);
    return buffer;
}

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
Function: Wakes up Touch Functionality
Parameters:
********************************************************************************/
void touch_wake_up()
{
    gpio_put(TOUCH_PIN_RST, 0);
    sleep_ms(10);
    gpio_put(TOUCH_PIN_RST, 1);
    sleep_ms(50);
    i2c_write(TOUCH_DISABLE_AUTO_SLEEP, 0x01);
}

/********************************************************************************
Function: Stops sleep mode
Parameters:
********************************************************************************/
void touch_stop_sleep()
{
    i2c_write(TOUCH_DISABLE_AUTO_SLEEP, 0x01);
}

/********************************************************************************
Function: Sets up Touch Functionality
Parameters:
        mode : mode to set touch functionality to...
********************************************************************************/
void touch_set_mode(uint8_t mode)
{
    if (mode == TOUCH_POINT_MODE)
    {
        i2c_write(TOUCH_IRQ_CONTROL, 0x41);
    }
    else if (mode == TOUCH_GESTURE_MODE)
    {
        i2c_write(TOUCH_IRQ_CONTROL, 0x11);
        i2c_write(TOUCH_MOTION_MASK, 0x01);
    }
    else
    {
        i2c_write(TOUCH_IRQ_CONTROL, 0x71);
    }
}

/********************************************************************************
Function: Initializes Touch Functionality
Parameters:
        mode : mode to set touch functionality to...
********************************************************************************/
void touch_init(uint8_t mode)
{
    touch_reset();

    // Do I need Who am I?

    touch_set_mode(mode);
    TOUCH.x_point = 0;
    TOUCH.y_point = 0;
    i2c_write(TOUCH_IRQ_PULSE_WIDTH, 0x01);
    i2c_write(TOUCH_NORMAL_SCAN_PERIOD, 0x01);

    TOUCH.mode = mode;

    return true;
}

/********************************************************************************
Function: Gets touch point
Parameters:
********************************************************************************/
touch_screen touch_get_point()
{
    uint8_t x_point_high, x_point_low; 
    uint8_t y_point_high, y_point_low;

    x_point_high = i2c_read(TOUCH_X_POS_HIGH);
    x_point_low = i2c_read(TOUCH_X_POS_LOW);
    y_point_high = i2c_read(TOUCH_Y_POS_HIGH);
    y_point_low = i2c_read(TOUCH_Y_POS_LOW);

    TOUCH.x_point = ((x_point_high & 0x0F) << 8) | x_point_low;
    TOUCH.y_point = ((y_point_high & 0x0F) << 8) | y_point_low;

    return TOUCH;
}

/********************************************************************************
Function: Returns current gesture
Parameters:
********************************************************************************/
uint8_t touch_get_gesture()
{
    uint8_t gesture;
    gesture = i2c_read(TOUCH_GESTURE_ID);
    return gesture;
}