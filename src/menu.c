
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

static pw_img_t* const MENU_ICONS[] = {
    &icon_mm_pokeradar,
    &icon_mm_dowsing,
    &icon_mm_connect,
    &icon_mm_trainer,
    &icon_mm_inventory,
    &icon_mm_settings,
};

static int8_t cursor = 0;

void pw_menu_init_display() {
    pw_screen_draw_img(MENU_TITLES[cursor], 8, 0);

    pw_screen_draw_img(&icon_mm_larrow, 0, 0);
    pw_screen_draw_img(&icon_mm_rarrow, SCREEN_WIDTH-8, 0);

    size_t y_values[] = {24, 26, 28, 30, 26, 24};
    for(size_t i = 0; i < MENU_SIZE; i++) {
        pw_screen_draw_img(MENU_ICONS[i], i*16, y_values[i]);
        if(cursor == i) {
            pw_screen_draw_img(&icon_arrow_down_normal, i*16, y_values[i]-8);
        } else {
            pw_screen_clear_area(i*16, y_values[i]-8, 8, 8);
        }
    }

    pw_screen_draw_img(&icon_watt_symbol, SCREEN_WIDTH-icon_watt_symbol.width,
            SCREEN_HEIGHT-icon_watt_symbol.height);
    pw_screen_draw_integer(2410, SCREEN_WIDTH-icon_watt_symbol.width,
            SCREEN_HEIGHT-icon_watt_symbol.height);


}

void pw_menu_handle_input(uint8_t b) {

	switch(b) {
		case BUTTON_L: { move_cursor(-1); break; };
		case BUTTON_M: { pw_request_state(MENU_ENTRIES[cursor]); break; };
		case BUTTON_R: { move_cursor(+1); break; };
		default: break;
	}

}

void pw_menu_update_display() {
    /*
     *  Redraw title, arrows
     */
    pw_screen_draw_img(MENU_TITLES[cursor], 8, 0);

    size_t y_values[] = {24, 26, 28, 30, 26, 24};
    for(size_t i = 0; i < MENU_SIZE; i++) {
        if(cursor == i) {
            pw_screen_draw_img(&icon_arrow_down_normal, i*16, y_values[i]-8);
        } else {
            pw_screen_clear_area(i*16, y_values[i]-8, 8, 8);
        }
    }

}

void pw_menu_set_cursor(int8_t c) {
	if( cursor < 0 || cursor >= MENU_SIZE ) {
		cursor = 0;
	} else {
		cursor = c;
	}
    //pw_request_redraw();

}


// + = right
// - = left
bool move_cursor(int8_t move) {
	cursor += move;

	if( cursor < 0 || cursor >= MENU_SIZE ) {
        cursor = 0;
		pw_request_state(STATE_SPLASH);
		return true;
	}

	cursor %= MENU_SIZE;
    pw_request_redraw();

	return false;
}

