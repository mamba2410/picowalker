#include <stdio.h>

#include "ir_comms.h"
#include "states.h"
#include "menu.h"
#include "buttons.h"
#include "screen.h"
#include "pico_roms.h"

#include "apps/app_splash.h"
#include "apps/app_trainer_card.h"
#include "apps/app_comms.h"
#include "apps/app_inventory.h"
#include "apps/app_dowsing.h"

const char* const state_strings[N_STATES] = {
    [STATE_SCREENSAVER]     = "Screensaver",
	[STATE_SPLASH]          = "Splash screen",
	[STATE_MAIN_MENU]       = "Main menu",
	[STATE_POKE_RADAR]      = "Poke radar",
	[STATE_DOWSING]         = "Dowsing",
	[STATE_CONNECT]         = "Connect",
	[STATE_TRAINER_CARD]    = "Trainer card",
	[STATE_INVENTORY]       = "Pokemon and items",
	[STATE_SETTINGS]        = "Settings",
    [STATE_ERROR]           = "Error",
};

state_event_func_t* const state_init_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_event,
	[STATE_SPLASH]          = pw_empty_event,
	[STATE_MAIN_MENU]       = pw_empty_event,
	[STATE_POKE_RADAR]      = pw_empty_event,
	[STATE_DOWSING]         = pw_empty_event,
	[STATE_CONNECT]         = pw_comms_init,
	[STATE_DOWSING]         = pw_dowsing_init,
	[STATE_CONNECT]         = pw_comms_init,
	[STATE_TRAINER_CARD]    = pw_trainer_card_init,
	[STATE_INVENTORY]       = pw_inventory_init,
	[STATE_SETTINGS]        = pw_empty_event,
    [STATE_ERROR]           = pw_empty_event,
};

state_event_func_t* const state_event_loop_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_event,
	[STATE_SPLASH]          = pw_empty_event,
	[STATE_MAIN_MENU]       = pw_empty_event,
	[STATE_POKE_RADAR]      = pw_empty_event,
	[STATE_DOWSING]         = pw_dowsing_event_loop,
	[STATE_CONNECT]         = pw_comms_event_loop,
	[STATE_TRAINER_CARD]    = pw_empty_event,
	[STATE_INVENTORY]       = pw_empty_event,
	[STATE_SETTINGS]        = pw_empty_event,
    [STATE_ERROR]           = pw_empty_event,
};

state_input_func_t* const state_input_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_input,
	[STATE_SPLASH]          = pw_splash_handle_input,
	[STATE_MAIN_MENU]       = pw_menu_handle_input,
	[STATE_POKE_RADAR]      = pw_send_to_error,
	[STATE_DOWSING]         = pw_dowsing_handle_input,
	[STATE_CONNECT]         = pw_comms_handle_input,
	[STATE_TRAINER_CARD]    = pw_trainer_card_handle_input,
	[STATE_INVENTORY]       = pw_inventory_handle_input,
	[STATE_SETTINGS]        = pw_send_to_error,
    [STATE_ERROR]           = pw_send_to_splash,
};

state_draw_func_t* const state_draw_init_funcs[] = {
    [STATE_SCREENSAVER]     = pw_screen_clear,
	[STATE_SPLASH]          = pw_splash_init_display,
	[STATE_MAIN_MENU]       = pw_menu_init_display,
	[STATE_POKE_RADAR]      = pw_screen_clear,
	[STATE_DOWSING]         = pw_dowsing_init_display,
	[STATE_CONNECT]         = pw_comms_init_display,
	[STATE_TRAINER_CARD]    = pw_trainer_card_init_display,
	[STATE_INVENTORY]       = pw_inventory_init_display,
	[STATE_SETTINGS]        = pw_screen_clear,
    [STATE_ERROR]           = pw_error_init_display,
};

state_draw_func_t* const state_draw_update_funcs[N_STATES] = {
    [STATE_SCREENSAVER]     = pw_empty_event,
	[STATE_SPLASH]          = pw_splash_update_display,
	[STATE_MAIN_MENU]       = pw_menu_update_display,
	[STATE_POKE_RADAR]      = pw_empty_event,
	[STATE_DOWSING]         = pw_dowsing_update_display,
	[STATE_CONNECT]         = pw_comms_draw_update,
	[STATE_TRAINER_CARD]    = pw_trainer_card_draw_update,
	[STATE_INVENTORY]       = pw_inventory_update_display,
	[STATE_SETTINGS]        = pw_empty_event,
    [STATE_ERROR]           = pw_empty_event,
};

static pw_state_t pw_current_state = STATE_SCREENSAVER;
static pw_state_t pw_requested_state = 0;
static uint32_t pw_requests = 0;

static state_vars_t global_statevars = {0,};


void pw_request_state(pw_state_t s_to) {
    PW_SET_REQUEST(pw_requests, PW_REQUEST_STATE_CHANGE);
    pw_requested_state = s_to;
}

void pw_request_redraw() {
    PW_SET_REQUEST(pw_requests, PW_REQUEST_REDRAW);
}

bool pw_set_state(pw_state_t s) {
	if(s > N_STATES || s < 0) {
		// reset
		return false;
	}

	pw_state_t prev_state = pw_current_state;

	if(s != pw_current_state) {
		pw_current_state = s;
        pw_screen_clear();
        state_init_funcs[s](&global_statevars);
        state_draw_init_funcs[s](&global_statevars);
		// TODO: Notify when changed to and from state
		return true;
	} else {
		printf("Warn: tried to change state to same state\n");
		return false;
	}
}


pw_state_t pw_get_state() {
	return pw_current_state;
}


//may not be needed/used
void pw_state_init() {
    state_init_funcs[pw_current_state](&global_statevars);
}

void pw_state_run_event_loop() {
    if(PW_GET_REQUEST(pw_requests, PW_REQUEST_STATE_CHANGE)) {
        pw_set_state(pw_requested_state);
        PW_CLR_REQUEST(pw_requests, PW_REQUEST_STATE_CHANGE);
        PW_CLR_REQUEST(pw_requests, PW_REQUEST_REDRAW);
    }

    state_event_loop_funcs[pw_current_state](&global_statevars);

    if(PW_GET_REQUEST(pw_requests, PW_REQUEST_REDRAW)) {
        pw_state_draw_update();
        PW_CLR_REQUEST(pw_requests, PW_REQUEST_REDRAW);
    }
}

void pw_state_handle_input(uint8_t b) {
    state_input_funcs[pw_current_state](&global_statevars, b);
}

void pw_state_draw_init() {
    state_draw_init_funcs[pw_current_state](&global_statevars);
}

void pw_state_draw_update() {
    global_statevars.anim_frame = !global_statevars.anim_frame;
    state_draw_update_funcs[pw_current_state](&global_statevars);
}

void pw_send_to_error(state_vars_t *sv, uint8_t b) {
    pw_request_state(STATE_ERROR);
}

void pw_send_to_splash(state_vars_t *sv, uint8_t b) {
    pw_request_state(STATE_SPLASH);
}


/*
 *  ========================================
 *  State functions
 *  ========================================
 */
void pw_empty_event(state_vars_t *sv) {}
void pw_empty_input(state_vars_t *sv, uint8_t b) {}

void pw_error_init_display(state_vars_t *sv) {
    pw_img_t sad_pokewalker_img   = {.height=48, .width=48, .data=sad_pokewalker, .size=576};
    pw_screen_draw_img(&sad_pokewalker_img, 0, 0);

}


