#ifndef PW_DRIVER_PICO_BUTTONS_H
#define PW_DRIVER_PICO_BUTTONS_H

#include <stdint.h>

#include "pico/types.h"

#define PIN_BUTTON_LEFT     13 // GP13, phys 17
#define PIN_BUTTON_MIDDLE   14 // GP14, phys 19
#define PIN_BUTTON_RIGHT    15 // CP15, phys 20

void pw_pico_setup_buttons();
void pw_pico_button_callback(uint gp, uint32_t events);

#endif /* PW_DRIVER_PICO_BUTTONS_H */
