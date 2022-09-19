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
    STATE_ERROR,
	N_STATES,
} pw_state_t;

typedef void (state_draw_func_t)();
typedef void (state_event_func_t)();
typedef void (state_input_func_t)(uint8_t);


/*
 *  Use an array of structures to represent each state.
 *  Each state should have:
 *  - name/string
 *  - initialiser function
 *  - event loop function
 *  - input handler function
 *  - draw initialiser function
 *  - draw update function
 */
extern const char* const state_strings[];
extern state_event_func_t* const state_init_funcs[];
extern state_event_func_t* const state_event_loop_funcs[];
extern state_input_func_t* const state_input_funcs[];
extern state_draw_func_t* const state_draw_init_funcs[];
extern state_draw_func_t* const state_draw_update_funcs[];

void pw_send_to_error(uint8_t b);
void pw_send_to_splash(uint8_t b);

bool pw_set_state(pw_state_t s);
pw_state_t pw_get_state();

void pw_state_init();
void pw_state_run_event_loop();
void pw_state_handle_input(uint8_t b);
void pw_state_draw_init();
void pw_state_draw_update();

/*
 *  State functions
 */
void pw_empty_event();
void pw_empty_input(uint8_t b);

// STATE_SPLASH
void pw_splash_init_display();
void pw_splash_handle_input(uint8_t b);

// STATE_ERROR
void pw_error_init_display();

#endif /* PW_STATES_H */

