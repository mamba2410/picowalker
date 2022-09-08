#ifndef PW_STATES_H
#define PW_STATES_H

#include <stdbool.h>
#include <stdint.h>

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

typedef void (state_draw_func_t)();

extern const char* const state_strings[];
extern state_draw_func_t* const state_draw_init_funcs[];
extern state_draw_func_t* const state_draw_update_funcs[];

bool pw_set_state(pw_state_t s);
pw_state_t pw_get_state();
void state_handle_button_press(pw_state_t s, uint8_t b);

void pw_splash_init_display();

#endif /* PW_STATES_H */

