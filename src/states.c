#include <stdio.h>

#include "states.h"
#include "menu.h"
#include "buttons.h"

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

bool pw_set_state(pw_state_t s) {
	if(s > N_STATES || s < 0) {
		// reset
		return false;
	}

	pw_state_t prev_state = pw_state;

	if(s != pw_state) {
		pw_state = s;
		printf("Changed state to %s\n", state_strings[s]);
		// TODO: Notify when changed to and from state
		return true;
	} else {
		printf("Error: tried to change state to same state\n");
		return false;
	}
}

pw_state_t pw_get_state() {
	return pw_state;
}


// TODO: Change to handle input
void state_handle_button_press(pw_state_t s, uint8_t b) {

	switch(s) {
		case STATE_SPLASH: { 
							pw_set_state(STATE_MAIN_MENU); 
							switch(b) {
								case BUTTON_M: { pw_menu_set_cursor((MENU_SIZE-1)/2); break; }
								case BUTTON_R: { pw_menu_set_cursor(MENU_SIZE-1); break; }
								case BUTTON_L: 
								default: { pw_menu_set_cursor(0); break; }
							}
							pw_menu_display(); 
							break;
						   };
		case STATE_MAIN_MENU: { pw_menu_handle_input(b); break; };
		default: {
					printf("Unhandled state\n");
				 	pw_set_state(STATE_SPLASH);
				 	break;
				 }
	}

}

