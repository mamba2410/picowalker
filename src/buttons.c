
#include "buttons.h"
#include "states.h"


#include "drivers/pico_buttons.h"

void pw_setup_buttons() {
    pw_pico_setup_buttons();
}

void pw_button_callback(uint8_t b) {
    pw_state_handle_input(b);
}


