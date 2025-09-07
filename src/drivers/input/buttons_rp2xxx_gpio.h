#ifndef PW_DRIVER_PICO_BUTTONS_H
#define PW_DRIVER_PICO_BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

#include "pico/types.h"

extern bool acknowledge_button_presses;

void pw_pico_setup_buttons();
void pw_pico_button_callback(uint gp, uint32_t events);

#endif /* PW_DRIVER_PICO_BUTTONS_H */
