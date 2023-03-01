
#include "buttons.h"
#include "states.h"

void pw_button_callback(uint8_t b) {
    pw_state_handle_input(b);
}


