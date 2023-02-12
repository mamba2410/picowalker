#include <stdint.h>

#include "app_splash.h"
#include "../states.h"
#include "../buttons.h"
#include "../menu.h"
#include "../trainer_info.h"
#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../screen.h"
#include "../utils.h"


void pw_splash_handle_input(state_vars_t *sv, uint8_t b) {
	switch(b) {
		case BUTTON_M: { pw_menu_set_cursor(sv, (MENU_SIZE-1)/2); break; }
		case BUTTON_L: { pw_menu_set_cursor(sv, MENU_SIZE-1); break; }
		case BUTTON_R:
		default: { pw_menu_set_cursor(sv, 0); break; }
	}
	pw_request_state(STATE_MAIN_MENU);
}

void pw_splash_init_display(state_vars_t *sv) {

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-64, 0,
        64, 48,
        PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2,
        PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME
    );

    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-24-16,
        32, 24,
        PW_EEPROM_ADDR_IMG_ROUTE_LARGE,
        PW_EEPROM_SIZE_IMG_ROUTE_LARGE
    );

    pokemon_summary_t caught_pokemon;
    for(uint8_t i = 0; i < 3; i++) {
        pw_eeprom_read(
            PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY+i*PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE,
            (uint8_t*)(&caught_pokemon),
            PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE
        );
        if(caught_pokemon.le_species != 0) {
            pw_screen_draw_from_eeprom(
                i*8, SCREEN_HEIGHT-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_BALL,
                PW_EEPROM_SIZE_IMG_BALL
            );
        }
    }

    //uint16_t *items = (uint16_t*)(&caught_pokemon); // re-use the memory
    struct {
        uint16_t le_item;
        uint16_t pad;
    } items[3];
    pw_eeprom_read(
        PW_EEPROM_ADDR_OBTAINED_ITEMS,
        //(uint8_t*)(&caught_pokemon),
        (uint8_t*)(items),
        PW_EEPROM_SIZE_OBTAINED_ITEMS
    );
    for(uint8_t i = 0; i < 3; i++) {
        if(items[i].le_item != 0) {
            pw_screen_draw_from_eeprom(
                24+i*8, SCREEN_HEIGHT-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_ITEM,
                PW_EEPROM_SIZE_IMG_ITEM
            );
        }
    }

    uint8_t special_inventory;
    pw_eeprom_read(
        PW_EEPROM_ADDR_RECEIVED_BITFIELD,
        &special_inventory,
        1
    );

    for(uint8_t i = 0; i < 4; i++) {
        if( (special_inventory&(1<<i)) ) {
            pw_screen_draw_from_eeprom(
                16+i*8, SCREEN_HEIGHT-16,
                8, 8,
                PW_EEPROM_ADDR_IMG_CARD_SUITS+i*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL,
                PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL
            );
        }

    }

    if( special_inventory&(1<<5) ) {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-16,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL_LIGHT,
            PW_EEPROM_SIZE_IMG_BALL_LIGHT
        );
    }

    pw_eeprom_read(
        PW_EEPROM_ADDR_EVENT_ITEM,
        (uint8_t*)items,
        PW_EEPROM_SIZE_EVENT_ITEM
    );
    if( ((uint16_t*)(items))[3] != 0) {     // Check fourth word because first 6 bytes are useless
        pw_screen_draw_from_eeprom(
            8, SCREEN_HEIGHT-16,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM_LIGHT,
            PW_EEPROM_SIZE_IMG_ITEM_LIGHT
        );
    }

    health_data_t health_data;
    int err = pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&health_data),
        PW_EEPROM_SIZE_HEALTH_DATA_1
    );
    uint32_t today_steps = swap_bytes_u32(health_data.be_today_steps);
    pw_screen_draw_integer(today_steps, SCREEN_WIDTH, SCREEN_HEIGHT-16);

    pw_screen_draw_horiz_line(0, SCREEN_HEIGHT-16, SCREEN_WIDTH, SCREEN_BLACK);
}

void pw_splash_update_display(state_vars_t *sv) {

    uint16_t frame_addr;
    if(sv->anim_frame) {
        frame_addr = PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME1;
    } else {
        frame_addr = PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2;
    }

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-64, 0,
        64, 48,
        frame_addr,
        PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME
    );

    health_data_t health_data;
    int err = pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&health_data),
        PW_EEPROM_SIZE_HEALTH_DATA_1
    );
    uint32_t today_steps = swap_bytes_u32(health_data.be_today_steps);
    pw_screen_draw_integer(today_steps, SCREEN_WIDTH, SCREEN_HEIGHT-16);
    pw_screen_draw_horiz_line(48, SCREEN_HEIGHT-16, SCREEN_WIDTH-48, SCREEN_BLACK);

}
