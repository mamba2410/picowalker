#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "buttons_pico.h"
#include "battery_pico_bq25628e.h"
#include "accel_pico_bma400.h"

#include "power_pico.h"
#include "../picowalker-defs.h"

void pw_gpio_interrupt_handler(uint gp, uint32_t events) {
    //printf("[Debug] interrupt on pin GP%d\n", gp);
    switch(gp) {
    case PIN_BUTTON_LEFT:
    case PIN_BUTTON_MIDDLE:
    case PIN_BUTTON_RIGHT: {
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
