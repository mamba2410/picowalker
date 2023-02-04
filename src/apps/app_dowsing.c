
#include "app_dowsing.h"

#include "../buttons.h"
#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../screen.h"
#include "../states.h"

#define BUSH_HEIGHT (SCREEN_HEIGHT-16-8-16)
#define ARROW_HEIGHT (SCREEN_HEIGHT-16-8)

/*
 *  sv.substate_a = item_position
 *  sv.substate_b = chosen_positions
 *  sv.substate_c = choices_remaining
 *  sv.substate_d = bush_shakes
 *
 */

static uint8_t img_buf[128];

static void switch_subscreen(state_vars_t *sv, uint8_t new) {
    sv->prev_subscreen = sv->subscreen;
    sv->subscreen = new;
}

static void move_cursor(state_vars_t *sv, int8_t m) {
    sv->prev_cursor = sv->cursor;
    sv->cursor += m;

    if(sv->cursor > 5) sv->cursor = 5;
    else if(sv->cursor < 0) sv->cursor = 0;
}

static uint16_t get_item(route_info_t *ri, health_data_t *hd) {
    uint32_t today_steps = swap_bytes_u32(hd->be_today_steps);
    uint8_t chosen = 0;

    do {
        chosen = rand()%10;
    } while(today_steps < ri->le_route_item_steps[chosen]);

    return ri->le_route_items[chosen];

}

void pw_dowsing_init(state_vars_t *sv) {
    // get steps
    // get items
    // choose item

    health_data_t hd;
    route_info_t ri;

    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)(&ri), PW_EEPROM_ADDR_ROUTE_INFO);


    sv->substate_a = 0b000001; // choose position

    sv->substate_c = 2; // set tries left
    sv->substate_d = 0;

    sv->cursor = sv->prev_cursor = 0;

}

void pw_dowsing_init_display(state_vars_t *sv) {
    switch(sv->subscreen) {
        case DOWSING_ENTRY: {

            pw_img_t grass = {.data=img_buf, .width=16, .height=16, .size=PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK};
            pw_eeprom_read(
                PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
                grass.data,
                PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
            );

            for(uint8_t i = 0; i < 6; i++) {
                pw_screen_draw_img(&grass, 16*i, BUSH_HEIGHT);
                if(i == sv->cursor) {
                    if(sv->anim_frame) {
                        pw_screen_draw_from_eeprom(
                            16*i+2, SCREEN_HEIGHT-16-8,
                            8, 8,
                            PW_EEPROM_ADDR_IMG_ARROW_UP_NORMAL,
                            PW_EEPROM_SIZE_IMG_ARROW
                        );
                    } else {
                        pw_screen_draw_from_eeprom(
                            16*i+2, SCREEN_HEIGHT-16-8,
                            8, 8,
                            PW_EEPROM_ADDR_IMG_ARROW_UP_OFFSET,
                            PW_EEPROM_SIZE_IMG_ARROW
                        );
                    }
                }
            }

            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                96, 16,
                PW_EEPROM_ADDR_TEXT_DISCOVER_ITEM,
                PW_EEPROM_SIZE_TEXT_DISCOVER_ITEM
            );

            pw_screen_draw_from_eeprom(
                0, 0,
                32, 24,
                PW_EEPROM_ADDR_IMG_ROUTE_LARGE,
                PW_EEPROM_SIZE_IMG_ROUTE_LARGE
            );

            pw_screen_draw_from_eeprom(
                36, 0,
                32, 16,
                PW_EEPROM_ADDR_TEXT_LEFT,
                PW_EEPROM_SIZE_TEXT_LEFT
            );

            pw_screen_draw_from_eeprom(
                76, 0,
                8, 16,
                PW_EEPROM_ADDR_IMG_DIGITS + PW_EEPROM_SIZE_IMG_CHAR*sv->substate_c,
                PW_EEPROM_SIZE_IMG_CHAR
            );
        }
        break;
    }

}

void pw_dowsing_update_display(state_vars_t *sv) {
    switch(sv->subscreen) {
        case DOWSING_CHOOSING: {
            pw_screen_clear_area(0, SCREEN_HEIGHT-16-8, SCREEN_WIDTH, 8);
            if(sv->anim_frame) {
                pw_screen_draw_from_eeprom(
                    16*sv->cursor+4, SCREEN_HEIGHT-16-8,
                    8, 8,
                    PW_EEPROM_ADDR_IMG_ARROW_UP_NORMAL,
                    PW_EEPROM_SIZE_IMG_ARROW
                );
            } else {
                pw_screen_draw_from_eeprom(
                    16*sv->cursor+4, SCREEN_HEIGHT-16-8,
                    8, 8,
                    PW_EEPROM_ADDR_IMG_ARROW_UP_OFFSET,
                    PW_EEPROM_SIZE_IMG_ARROW
                );
            }
            break;
        }
        case DOWSING_SELECTED: {
            sv->substate_d++;
            pw_screen_clear_area(16*sv->cursor, ARROW_HEIGHT, 8, 8);
            if(sv->anim_frame) {
                pw_screen_draw_from_eeprom(
                    16*sv->cursor, BUSH_HEIGHT+2,
                    16, 16,
                    PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
                    PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
                );
                pw_screen_clear_area(16*sv->cursor, BUSH_HEIGHT, 16, 2);
            } else {
                pw_screen_draw_from_eeprom(
                    16*sv->cursor, BUSH_HEIGHT-2,
                    16, 16,
                    PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
                    PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
                );
                pw_screen_clear_area(16*sv->cursor, BUSH_HEIGHT+16-2, 16, 2);
            }
        }
        default: break;
    }
}

void pw_dowsing_handle_input(state_vars_t *sv, uint8_t b) {
    switch(sv->subscreen) {
        case DOWSING_CHOOSING: {
            switch(b) {
                case BUTTON_L: move_cursor(sv, -1); break;
                case BUTTON_R: move_cursor(sv, +1); break;
                case BUTTON_M: switch_subscreen(sv, DOWSING_SELECTED); break;
            }
            break;
        }
        default: break;
    }
}

void pw_dowsing_event_loop(state_vars_t *sv) {

    switch(sv->subscreen) {
        case DOWSING_ENTRY: switch_subscreen(sv, DOWSING_CHOOSING); break;
        case DOWSING_SELECTED: {
            // after 4 frames, set substate check correct
            if(sv->substate_d >= 4) {
                switch_subscreen(sv, DOWSING_CHECK_GUESS);
            }
            break;
        }
        case DOWSING_CHECK_GUESS: {
            sv->substate_d = 0;
            sv->substate_c--;
            // TODO: draw new guesses left
            if(sv->substate_a & sv->substate_b) {
                switch_subscreen(sv, DOWSING_GIVE_ITEM);
            } else {
                // display nothing found in update_display
                if(sv->substate_d > 0) {
                    switch_subscreen(sv, DOWSING_INTERMEDIATE);
                    sv->substate_d = 0;
                }
            }
            break;
        }
        case DOWSING_INTERMEDIATE: {
            // display near/far
            switch_subscreen(sv, DOWSING_CHOOSING);
            break;
        }
        case DOWSING_GIVE_ITEM: {
            if( 0 /* inv full */ ) {
                switch_subscreen(sv, DOWSING_REPLACE_ITEM);
            } else {
                // give item
                switch_subscreen(sv, DOWSING_QUITTING);
            }
            break;
        }
        case DOWSING_REPLACE_ITEM: {
            // replace it idk
            break;
        }
        case DOWSING_CHOOSING: break;
        default:
            break;
    }

}

