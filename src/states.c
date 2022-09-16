#include <stdio.h>

#include "ir_comms.h"
#include "states.h"
#include "menu.h"
#include "buttons.h"
#include "screen.h"

#include "apps/app_trainer_card.h"

#include "pwroms.h"

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
    "Error",
};

state_draw_func_t* const state_draw_init_funcs[] = {
    pw_screen_clear,
    pw_splash_init_display,
    pw_menu_init_display,
    pw_screen_clear,
    pw_screen_clear,
    pw_screen_clear,
    pw_trainer_card_init_display,
    pw_screen_clear,
    pw_screen_clear,
    pw_error_init_display,
};

static pw_state_t pw_state = STATE_SCREENSAVER;

bool pw_set_state(pw_state_t s) {
	if(s > N_STATES || s < 0) {
		// reset
		return false;
	}

	pw_state_t prev_state = pw_state;

	if(s != pw_state) {
		pw_state = s;
        pw_screen_clear();
        state_draw_init_funcs[s]();
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


/*
 *	Triggers when button is pressed.
 *	Want to spend as little time as possible in here
 */
void state_handle_button_press(pw_state_t s, uint8_t b) {

	switch(s) {
		case STATE_SPLASH:
			switch(b) {
				case BUTTON_M: { pw_menu_set_cursor((MENU_SIZE-1)/2); break; }
				case BUTTON_L: { pw_menu_set_cursor(MENU_SIZE-1); break; }
				case BUTTON_R:
				default: { pw_menu_set_cursor(0); break; }
			}
			pw_set_state(STATE_MAIN_MENU);
			break;
		case STATE_MAIN_MENU:
            pw_menu_handle_input(b);
            break;

        case STATE_TRAINER_CARD:
            pw_trainer_card_handle_input(b);
		case STATE_CONNECT:
			 //if( ir_get_state() == COMM_IDLE )
			 //	ir_set_state(COMM_ADVERTISING);
        case STATE_ERROR:
            pw_set_state(STATE_SPLASH);
            break;
		default:
			printf("Unhandled state\n");
			pw_set_state(STATE_ERROR);
			break;
	}

	// Return from here to exit interrupt and back to main loop
	// TODO: Have a way to set main loop behaviour
}

void pw_splash_init_display() {
    pw_screen_draw_img(&img_pokemon_large_frame1, SCREEN_WIDTH-img_pokemon_large_frame1.width, 0);
    pw_screen_draw_img(&img_route, 0, SCREEN_HEIGHT-img_route.height-16);

}

void pw_error_init_display() {
    pw_img_t sad_pokewalker_img   = {.height=48, .width=48, .data=sad_pokewalker, .size=576};
    pw_screen_draw_img(&sad_pokewalker_img, 0, 0);

}


