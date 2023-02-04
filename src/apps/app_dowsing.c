
#include "app_dowsing.h"

#include "../buttons.h"
#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../screen.h"
#include "../states.h"
#include "../rand.h"
#include "../trainer_info.h"
#include "../route_info.h"
#include "../utils.h"

#define BUSH_HEIGHT (SCREEN_HEIGHT-16-8-16)
#define ARROW_HEIGHT (SCREEN_HEIGHT-16-8)

/*
 *  sv.reg_a = item_position
 *  sv.reg_b = chosen_positions
 *  sv.reg_c = choices_remaining
 *  sv.reg_d = bush_shakes
 *
 */

static uint8_t img_buf[128];
static uint16_t le_chosen;
static uint8_t  le_chosen_idx;

static void switch_substate(state_vars_t *sv, uint8_t new) {
    sv->substate_2 = sv->current_substate;
    sv->current_substate = new;
}

static void move_cursor(state_vars_t *sv, int8_t m) {
    sv->cursor_2 = sv->current_cursor;
    sv->current_cursor += m;

    if(sv->current_cursor > 5) sv->current_cursor = 5;
    else if(sv->current_cursor < 0) sv->current_cursor = 0;
}

static uint16_t get_item(route_info_t *ri, health_data_t *hd) {
    uint32_t today_steps = swap_bytes_u32(hd->be_today_steps);

    // TODO: checks for gift item

    do {
        le_chosen_idx = pw_rand()%10;
    } while(today_steps < ri->le_route_item_steps[le_chosen_idx]);

    return ri->le_route_items[le_chosen_idx];
}


void pw_dowsing_init(state_vars_t *sv) {
    health_data_t hd;
    route_info_t ri;

    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)(&ri), PW_EEPROM_SIZE_ROUTE_INFO);
    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&hd),
        PW_EEPROM_SIZE_HEALTH_DATA_1
    );

    uint16_t le_chosen = get_item(&ri, &hd);

    sv->reg_a = 0b000001; // choose position
    //sv->reg_a = 1<<(pw_rand()%6);

    sv->reg_c = 2; // set tries left
    sv->reg_d = 0;

    sv->current_cursor = sv->cursor_2 = 0;

}

void pw_dowsing_init_display(state_vars_t *sv) {
    switch(sv->current_substate) {
        case DOWSING_ENTRY: {

            pw_img_t grass = {.data=img_buf, .width=16, .height=16, .size=PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK};
            pw_eeprom_read(
                PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
                grass.data,
                PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
            );

            for(uint8_t i = 0; i < 6; i++) {
                pw_screen_draw_img(&grass, 16*i, BUSH_HEIGHT);
                if(i == sv->current_cursor) {
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
                PW_EEPROM_ADDR_IMG_DIGITS + PW_EEPROM_SIZE_IMG_CHAR*sv->reg_c,
                PW_EEPROM_SIZE_IMG_CHAR
            );
        }
        break;
    }

}

void pw_dowsing_update_display(state_vars_t *sv) {
    switch(sv->current_substate) {
        case DOWSING_CHOOSING: {
            pw_screen_clear_area(0, SCREEN_HEIGHT-16-8, SCREEN_WIDTH, 8);
            if(sv->anim_frame) {
                pw_screen_draw_from_eeprom(
                    16*sv->current_cursor+4, SCREEN_HEIGHT-16-8,
                    8, 8,
                    PW_EEPROM_ADDR_IMG_ARROW_UP_NORMAL,
                    PW_EEPROM_SIZE_IMG_ARROW
                );
            } else {
                pw_screen_draw_from_eeprom(
                    16*sv->current_cursor+4, SCREEN_HEIGHT-16-8,
                    8, 8,
                    PW_EEPROM_ADDR_IMG_ARROW_UP_OFFSET,
                    PW_EEPROM_SIZE_IMG_ARROW
                );
            }
            break;
        }
        case DOWSING_SELECTED: {
            sv->reg_d++;
            pw_screen_clear_area(16*sv->current_cursor, ARROW_HEIGHT, 8, 8);
            if(sv->anim_frame) {
                pw_screen_draw_from_eeprom(
                    16*sv->current_cursor, BUSH_HEIGHT+2,
                    16, 16,
                    PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
                    PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
                );
                pw_screen_clear_area(16*sv->current_cursor, BUSH_HEIGHT, 16, 2);
            } else {
                pw_screen_draw_from_eeprom(
                    16*sv->current_cursor, BUSH_HEIGHT-2,
                    16, 16,
                    PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
                    PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
                );
                pw_screen_clear_area(16*sv->current_cursor, BUSH_HEIGHT+16-2, 16, 2);
            }
        }
        default: break;
    }
}

void pw_dowsing_handle_input(state_vars_t *sv, uint8_t b) {
    switch(sv->current_substate) {
        case DOWSING_CHOOSING: {
            switch(b) {
                case BUTTON_L: move_cursor(sv, -1); break;
                case BUTTON_R: move_cursor(sv, +1); break;
                case BUTTON_M: switch_substate(sv, DOWSING_SELECTED); break;
            }
            break;
        }
        case DOWSING_INTERMEDIATE:
        case DOWSING_CHECK_GUESS: {
            sv->reg_d = 1; // we have pressed a button
            break;
        }
        default: break;
    }
}

void pw_dowsing_event_loop(state_vars_t *sv) {

    switch(sv->current_substate) {
        case DOWSING_ENTRY: switch_substate(sv, DOWSING_CHOOSING); break;
        case DOWSING_SELECTED: {
            // after 4 frames, set substate check correct
            if(sv->reg_d >= 4) {
                switch_substate(sv, DOWSING_CHECK_GUESS);
            }
            break;
        }
        case DOWSING_CHECK_GUESS: {
            // Hacky way to separategetting input
            if(sv->reg_d == 4) {
                sv->reg_d = 0;
                sv->reg_c--;

                pw_screen_draw_from_eeprom(
                    76, 0,
                    8, 16,
                    PW_EEPROM_ADDR_IMG_DIGITS + PW_EEPROM_SIZE_IMG_CHAR*sv->reg_c,
                    PW_EEPROM_SIZE_IMG_CHAR
                );

                if(sv->reg_a & sv->reg_b) {
                    switch_substate(sv, DOWSING_GIVE_ITEM);
                } else {
                    pw_screen_draw_from_eeprom(
                        0, SCREEN_HEIGHT-16,
                        96, 16,
                        PW_EEPROM_ADDR_TEXT_NOTHING_FOUND,
                        PW_EEPROM_SIZE_TEXT_NOTHING_FOUND
                    );
                }

            } else {
                // wait for user input on reg_d before switching
                if(sv->reg_d > 0) {
                    switch_substate(sv, DOWSING_INTERMEDIATE);
                    sv->reg_d = 0;
                }
            }
            break;
        }
        case DOWSING_INTERMEDIATE: {

            // TODO: do this properly
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                96, 16,
                PW_EEPROM_ADDR_TEXT_ITS_NEAR,
                PW_EEPROM_SIZE_TEXT_ITS_NEAR
            );

            if(sv->reg_d > 0) {
                switch_substate(sv, DOWSING_CHOOSING);
                sv->reg_d = 0;
            }
            break;
        }
        case DOWSING_GIVE_ITEM: {
            if( 0 /* inv full */ ) {
                switch_substate(sv, DOWSING_REPLACE_ITEM);
            } else {
                // give item
                switch_substate(sv, DOWSING_QUITTING);
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

