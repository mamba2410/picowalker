#ifndef PW_DRIVER_PICO_BUTTONS_H
#define PW_DRIVER_PICO_BUTTONS_H

#include <stdint.h>

#include "pico/types.h"

#define PIN_BUTTON_LEFT     26 // GP20, phys 31
#define PIN_BUTTON_MIDDLE   22 // GP11, phys 29
#define PIN_BUTTON_RIGHT    19 // GP19, phys 25

void pw_pico_setup_buttons();
void pw_pico_button_callback(uint gp, uint32_t events);

#endif /* PW_DRIVER_PICO_BUTTONS_H */
