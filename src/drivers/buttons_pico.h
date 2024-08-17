#ifndef PW_DRIVER_PICO_BUTTONS_H
#define PW_DRIVER_PICO_BUTTONS_H

#include <stdint.h>

#include "pico/types.h"

#define PIN_BUTTON_LEFT     10 // GP10, phys 14
#define PIN_BUTTON_MIDDLE   11 // GP11, phys 15
#define PIN_BUTTON_RIGHT    12 // CP12, phys 26

void pw_pico_setup_buttons();
void pw_pico_button_callback(uint gp, uint32_t events);

#endif /* PW_DRIVER_PICO_BUTTONS_H */
