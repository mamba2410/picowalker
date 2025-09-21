#ifndef PW_DRIVER_PICO_BUTTONS_H
#define PW_DRIVER_PICO_BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

#include "pico/types.h"

<<<<<<< HEAD:src/drivers/buttons/buttons_pico.h
<<<<<<< HEAD
#define PIN_BUTTON_LEFT     17
#define PIN_BUTTON_MIDDLE   16
#define PIN_BUTTON_RIGHT    26
=======
#define PIN_BUTTON_LEFT     26 // GP20, phys 31
#define PIN_BUTTON_MIDDLE   22 // GP11, phys 29
#define PIN_BUTTON_RIGHT    19 // GP19, phys 25
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)

=======
>>>>>>> 5cad753 (rebase survival):src/drivers/input/buttons_rp2xxx_gpio.h
extern bool acknowledge_button_presses;

void pw_pico_setup_buttons();
void pw_pico_button_callback(uint gp, uint32_t events);

#endif /* PW_DRIVER_PICO_BUTTONS_H */