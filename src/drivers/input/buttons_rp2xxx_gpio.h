#ifndef PW_DRIVER_PICO_BUTTONS_H
#define PW_DRIVER_PICO_BUTTONS_H

#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>

extern bool acknowledge_button_presses;

void pw_pico_button_callback(uint gp, uint32_t events);

#endif /* PW_DRIVER_PICO_BUTTONS_H */
