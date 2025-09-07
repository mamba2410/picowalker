#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "board_resources.h"
#include "../input/buttons_rp2xxx_gpio.h"
#include "../battery/bq25628e_rp2xxx_i2c.h"
#include "../accel/bma400_rp2xxx_spi.h"
#include "../sleep/dormant_rp2xxx.h"
#include "../../picowalker-defs.h"

void pw_gpio_interrupt_handler(uint gp, uint32_t events) {
    //printf("[Debug] interrupt on pin GP%d\n", gp);
    switch(gp) {
    case BUTTON_LEFT_PIN:
    case BUTTON_MIDDLE_PIN:
    case BUTTON_RIGHT_PIN: {
        wake_reason |= PW_WAKE_REASON_BUTTON;
        pw_pico_button_callback(gp, events);
        break;
    }
    case BAT_INT_PIN: {
        wake_reason |= PW_WAKE_REASON_BATTERY;
        pw_battery_int(gp, events);
        break;
    }
    case ACCEL_INT_PIN: {
        //printf("[Debug] Accel interrupt\n");
        //pw_accel_reset_int();
        wake_reason |= PW_WAKE_REASON_ACCEL;
        break;
    }
    default: {
        printf("[Warn] Unhandled interrupt on pin GP%d\n", gp);
        break;
    }
    }
}
