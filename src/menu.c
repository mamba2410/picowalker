
#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "states.h"
#include "buttons.h"
#include "pw_images.h"
#include "screen.h"

static pw_state_t const MENU_ENTRIES[] = {
	STATE_POKE_RADAR,
	STATE_DOWSING,
	STATE_CONNECT,
	STATE_TRAINER_CARD,
	STATE_INVENTORY,
	STATE_SETTINGS,
};
const int8_t MENU_SIZE = sizeof(MENU_ENTRIES);

static pw_img_t* const MENU_TITLES[] = {
    &text_mm_pokeradar,
    &text_mm_dowsing,
    &text_mm_connect,
    &text_mm_trainer,
    &text_mm_inventory,
    &text_mm_settings,
};

static int8_t cursor = 0;

void pw_menu_init_display() {
    pw_screen_draw_img(MENU_TITLES[cursor], 0, 0);
}

void pw_menu_handle_input(uint8_t b) {

	bool changed_state = false;

	switch(b) {
		case BUTTON_L: { changed_state = move_cursor(-1); break; };
		case BUTTON_M: { changed_state = pw_set_state(MENU_ENTRIES[cursor]); break; };
		case BUTTON_R: { changed_state = move_cursor(+1); break; };
		default: break;
	}

    // if we're still on the menu, update the display
	if(!changed_state)
		pw_menu_init_display();

}

void pw_menu_set_cursor(int8_t c) {
	if( cursor < 0 || cursor >= MENU_SIZE ) {
		cursor = 0;
	} else {
		cursor = c;
	}

}


// + = right
// - = left
bool move_cursor(int8_t move) {
	cursor += move;

	if( cursor < 0 || cursor >= MENU_SIZE ) {
		pw_set_state(STATE_SPLASH);
		return true;
	}

	cursor %= MENU_SIZE;

	return false;
}

