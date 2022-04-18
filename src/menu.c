
#include <stdio.h>

#include "menu.h"
#include "states.h"
#include "buttons.h"

static pw_state_t const MENU_ENTRIES[] = {
	STATE_POKE_RADAR,
	STATE_DOWSING,
	STATE_CONNECT,
	STATE_TRAINER_CARD,
	STATE_INVENTORY,
	STATE_SETTINGS,
};
const int8_t MENU_SIZE = sizeof(MENU_ENTRIES);

// TODO: remove when using actual graphics
static char* const MENU_TITLES[] = {
	"Poke Radar",
	"Dowsing",
	"Connect",
	"Trainer Card",
	"Pokemon and Items",
	"Settings",
};

static int8_t cursor = 0;

void pw_menu_display() {
	printf("Menu:\n");
	for(int8_t i = 0; i < MENU_SIZE; i++) {
		printf("%c %s\n", 
				(i==cursor)?'>':' ',
				MENU_TITLES[i]);
	}
	printf("\n");
}

void pw_menu_handle_input(uint8_t b) {

	bool changed_state = false;

	switch(b) {
		case BUTTON_L: { changed_state = move_cursor(-1); break; };
		case BUTTON_M: { changed_state = pw_set_state(MENU_ENTRIES[cursor]); break; };
		case BUTTON_R: { changed_state = move_cursor(+1); break; };
		default: break;
	}

	if(!changed_state)
		pw_menu_display();

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

