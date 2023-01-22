#ifndef PW_BUTTONS_H
#define PW_BUTTONS_H

#include <stdint.h>

// Add an input type? For htings that aren't just buttons
enum {
	BUTTON_L = 0x01,
	BUTTON_M = 0x02,
	BUTTON_R = 0x04,
};

void pw_setup_buttons();
void pw_button_callback(uint8_t b);

#endif /* PW_BUTTONS_H */
