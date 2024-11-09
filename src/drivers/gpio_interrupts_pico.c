#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "buttons_pico.h"
#include "battery_pico_bq25628e.h"

void pw_gpio_interrupt_handler(uint gp, uint32_t events) {
    switch(gp) {
    case PIN_BUTTON_LEFT:
    case PIN_BUTTON_MIDDLE:
    case PIN_BUTTON_RIGHT: {
        pw_pico_button_callback(gp, events);
        break;
    }
    case BAT_INT_PIN: {
        pw_battery_int(gp, events);
        break;
    }
    default: {
        printf("[Warn] Unhandled interrupt on pin GP%d\n", gp);
        break;
    }
    }
}
