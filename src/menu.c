
#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "states.h"
#include "buttons.h"
#include "pw_images.h"
#include "screen.h"
#include "utils.h"
#include "trainer_info.h"
#include "eeprom_map.h"
#include "eeprom.h"

static pw_state_t const MENU_ENTRIES[] = {
	STATE_POKE_RADAR,
	STATE_DOWSING,
	STATE_CONNECT,
	STATE_TRAINER_CARD,
	STATE_INVENTORY,
	STATE_SETTINGS,
};
const int8_t MENU_SIZE = sizeof(MENU_ENTRIES);

static uint16_t const MENU_TITLES[] = {
    PW_EEPROM_ADDR_IMG_MENU_TITLE_POKERADAR,
    PW_EEPROM_ADDR_IMG_MENU_TITLE_DOWSING,
    PW_EEPROM_ADDR_IMG_MENU_TITLE_CONNECT,
    PW_EEPROM_ADDR_IMG_MENU_TITLE_TRAINER_CARD,
    PW_EEPROM_ADDR_IMG_MENU_TITLE_INVENTORY,
    PW_EEPROM_ADDR_IMG_MENU_TITLE_SETTINGS,
};

static uint16_t const MENU_ICONS[] = {
    PW_EEPROM_ADDR_IMG_MENU_ICON_POKERADAR,
    PW_EEPROM_ADDR_IMG_MENU_ICON_DOWSING,
    PW_EEPROM_ADDR_IMG_MENU_ICON_CONNECT,
    PW_EEPROM_ADDR_IMG_MENU_ICON_TRAINER_CARD,
    PW_EEPROM_ADDR_IMG_MENU_ICON_INVENTORY,
    PW_EEPROM_ADDR_IMG_MENU_ICON_SETTINGS,
};

void pw_menu_init_display(state_vars_t *sv) {

    pw_screen_draw_from_eeprom(
        8, 0,
        80, 16,
        MENU_TITLES[sv->cursor],
        PW_EEPROM_SIZE_IMG_MENU_TITLE_CONNECT
    );
    pw_screen_draw_from_eeprom(
        0, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_LEFT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_LEFT
    );
    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-8, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RIGHT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RIGHT
    );

    size_t y_values[] = {24, 26, 28, 30, 26, 24};
    for(size_t i = 0; i < MENU_SIZE; i++) {
        pw_screen_draw_from_eeprom(
            i*16, y_values[i],
            16, 16,
            MENU_ICONS[i],
            PW_EEPROM_SIZE_IMG_MENU_ICON_CONNECT
        );

        if(sv->cursor == i) {
            pw_screen_draw_from_eeprom(
                i*16, y_values[i]-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL,
                PW_EEPROM_SIZE_IMG_ARROW
            );
        } else {
            pw_screen_clear_area(i*16, y_values[i]-8, 8, 8);
        }
    }

    health_data_t health_data;
    int err = pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&health_data),
        PW_EEPROM_SIZE_HEALTH_DATA_1
    );
    const uint16_t current_watts = swap_bytes_u16(health_data.be_current_watts);;

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-16, SCREEN_HEIGHT-16,
        16, 16,
        PW_EEPROM_ADDR_IMG_WATTS,
        PW_EEPROM_SIZE_IMG_WATTS
    );
    pw_screen_draw_integer(current_watts, SCREEN_WIDTH-16, SCREEN_HEIGHT-16);

}

void pw_menu_handle_input(state_vars_t *sv, uint8_t b) {

	switch(b) {
		case BUTTON_L: { pw_menu_move_cursor(sv, -1); break; };
		case BUTTON_M: { pw_request_state(MENU_ENTRIES[sv->cursor]); break; };
		case BUTTON_R: { pw_menu_move_cursor(sv, +1); break; };
		default: break;
	}

}

void pw_menu_update_display(state_vars_t *sv) {
    /*
     *  Redraw title, arrows
     */
    pw_screen_draw_from_eeprom(
        8, 0,
        80, 16,
        MENU_TITLES[sv->cursor],
        PW_EEPROM_SIZE_IMG_MENU_TITLE_CONNECT
    );

    size_t y_values[] = {24, 26, 28, 30, 26, 24};
    for(size_t i = 0; i < MENU_SIZE; i++) {
        if(sv->cursor == i) {
            pw_screen_draw_from_eeprom(
                i*16, y_values[i]-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL,
                PW_EEPROM_SIZE_IMG_ARROW
            );

        } else {
            pw_screen_clear_area(i*16, y_values[i]-8, 8, 8);
        }
    }

}

void pw_menu_set_cursor(state_vars_t *sv, int8_t c) {
	if( c < 0 || c >= MENU_SIZE ) {
		sv->cursor = 0;
	} else {
		sv->cursor = c;
	}
    //pw_request_redraw();

}


// + = right
// - = left
bool pw_menu_move_cursor(state_vars_t *sv, int8_t move) {
	sv->cursor += move;

	if( sv->cursor < 0 || sv->cursor >= MENU_SIZE ) {
        sv->cursor = 0;
		pw_request_state(STATE_SPLASH);
		return true;
	}

	sv->cursor %= MENU_SIZE;
    pw_request_redraw();

	return false;
}

