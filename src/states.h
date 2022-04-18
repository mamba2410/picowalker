#ifndef PW_STATES_H
#define PW_STATES_H

typedef enum {
	STATE_SCREENSAVER,
	STATE_SPLASH,
	STATE_MAIN_MENU,
	STATE_POKE_RADAR,
	STATE_DOWSING,
	STATE_CONNECT,
	STATE_TRAINER_CARD,
	STATE_INVENTORY,
	STATE_SETTINGS,
	N_STATES,
} pw_state_t;

extern const char* const state_strings[];

void pw_set_state(pw_state_t s);
pw_state_t pw_get_state();
void state_handle_button_press(pw_state_t s, uint8_t b);

#endif /* PW_STATES_H */

