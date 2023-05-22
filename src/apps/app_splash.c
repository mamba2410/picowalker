#include <stdint.h>

#include "app_splash.h"
#include "../states.h"
#include "../buttons.h"
#include "../menu.h"
#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../screen.h"
#include "../utils.h"
#include "../types.h"
#include "../globals.h"

/// @file app_splash.c


void pw_splash_init(state_vars_t *sv) {
    pw_read_inventory(sv);
}

void pw_splash_handle_input(state_vars_t *sv, uint8_t b) {
    switch(b) {
    case BUTTON_M: {
        pw_menu_set_cursor(sv, (MENU_SIZE-1)/2);
        break;
    }
    case BUTTON_L: {
        pw_menu_set_cursor(sv, MENU_SIZE-1);
        break;
    }
    case BUTTON_R:
    default: {
        pw_menu_set_cursor(sv, 0);
        break;
    }
    }
    pw_request_state(STATE_MAIN_MENU);
}

void pw_splash_init_display(state_vars_t *sv) {


    if(sv->reg_a & INV_WALKING_POKEMON) {
        pw_screen_draw_from_eeprom(
            SCREEN_WIDTH-64, 0,
            64, 48,
            PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2,
            PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME
        );
    }

    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-24-16,
        32, 24,
        PW_EEPROM_ADDR_IMG_ROUTE_LARGE,
        PW_EEPROM_SIZE_IMG_ROUTE_LARGE
    );

    for(uint8_t i = 0; i < 3; i++) {
        if(sv->reg_a & (1<<(i+1))) {
            pw_screen_draw_from_eeprom(
                i*8, SCREEN_HEIGHT-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_BALL,
                PW_EEPROM_SIZE_IMG_BALL
            );
        }
    }

    for(uint8_t i = 0; i < 3; i++) {
        if(sv->reg_b & (1<<(i+1))) {
            pw_screen_draw_from_eeprom(
                24+i*8, SCREEN_HEIGHT-8,
                8, 8,
                PW_EEPROM_ADDR_IMG_ITEM,
                PW_EEPROM_SIZE_IMG_ITEM
            );
        }
    }


    for(uint8_t i = 0; i < 4; i++) {
        if( (sv->reg_c&(1<<i)) ) {
            pw_screen_draw_from_eeprom(
                16+i*8, SCREEN_HEIGHT-16,
                8, 8,
                PW_EEPROM_ADDR_IMG_CARD_SUITS+i*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL,
                PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL
            );
        }

    }

    if( sv->reg_a&INV_EVENT_POKEMON ) {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-16,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL_LIGHT,
            PW_EEPROM_SIZE_IMG_BALL_LIGHT
        );
    }

    if( sv->reg_b&INV_FOUND_EVENT_ITEM ) {
        pw_screen_draw_from_eeprom(
            8, SCREEN_HEIGHT-16,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM_LIGHT,
            PW_EEPROM_SIZE_IMG_ITEM_LIGHT
        );
    }

    pw_screen_draw_integer(health_data_cache.today_steps, SCREEN_WIDTH, SCREEN_HEIGHT-16);
    pw_screen_draw_horiz_line(0, SCREEN_HEIGHT-16, SCREEN_WIDTH, SCREEN_BLACK);
}

void pw_splash_update_display(state_vars_t *sv) {

    uint16_t frame_addr;
    if(sv->anim_frame&ANIM_FRAME_NORMAL_TIME) {
        frame_addr = PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME1;
    } else {
        frame_addr = PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2;
    }

    if(sv->reg_a & INV_WALKING_POKEMON) {
        pw_screen_draw_from_eeprom(
            SCREEN_WIDTH-64, 0,
            64, 48,
            frame_addr,
            PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME
        );

    }

    pw_screen_draw_integer(health_data_cache.today_steps, SCREEN_WIDTH, SCREEN_HEIGHT-16);
    pw_screen_draw_horiz_line(48, SCREEN_HEIGHT-16, SCREEN_WIDTH-48, SCREEN_BLACK);

}

