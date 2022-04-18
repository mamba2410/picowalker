#include <stdio.h>

#include "states.h"

const char* const state_strings[] = {
	"Screensaver",
	"Splash screen",
	"Main menu",
	"Poke radar",
	"Dowsing",
	"Connect",
	"Trainer card",
	"Pokemon and items",
	"Settings",
};

static pw_state_t pw_state = STATE_SPLASH;

void pw_set_state(pw_state_t s) {
	if(s > N_STATES || s < 0) {
		// reset
		return;
	}

	char buf[32];

	if(s != pw_state) {
		pw_state = s;
		sprintf(buf, "Changed state to %s\n", state_strings[s]);
		printf(buf);
		// add on state change
	} else {
		printf("Error: tried to change state to same state\n");
	}

}

pw_state_t pw_get_state() {
	return pw_state;
}


// TODO: Change to handle input
void state_handle_button_press(pw_state_t s, uint8_t b) {

	switch(s) {
		case STATE_SPLASH: { pw_set_state(STATE_MAIN_MENU); break; }
		default: break;
	}

}

