
#include "app_dowsing.h"

#include "../buttons.h"
#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../screen.h"
#include "../states.h"
#include "../rand.h"
#include "../utils.h"
#include "../types.h"
#include "../globals.h"

/** @file app_dowsing.c
 * ```
 *  sv.reg_a = item_position
 *  sv.reg_b = chosen_positions
 *  sv.reg_c = choices_remaining
 *  sv.reg_d = bush_shakes/user input
 *  sv.reg_x = chosen item
 *  sv.reg_y = chosen item index
 * ```
 *
 */

#define BUSH_HEIGHT (SCREEN_HEIGHT-16-8-16)
#define ARROW_HEIGHT (SCREEN_HEIGHT-16-8)


static uint8_t img_buf[128];
static void check_guess_draw_init(state_vars_t *sv);
static void replace_item_draw_update(state_vars_t *sv);
static void replace_item_draw_init(state_vars_t *sv);
static void selected_draw_update(state_vars_t *sv);
static void choosing_draw_update(state_vars_t *sv);
static void choosing_draw_init(state_vars_t *sv);

static void match_substate_2(state_vars_t *sv) {
    sv->substate_2 = sv->current_substate;
}

draw_func_t* const draw_init_funcs[N_DOWSING_STATES] = {
    [DOWSING_ENTRY]         = match_substate_2,
    [DOWSING_CHOOSING]      = choosing_draw_init,
    [DOWSING_SELECTED]      = match_substate_2,
    [DOWSING_INTERMEDIATE]  = match_substate_2,
    [DOWSING_CHECK_GUESS]   = check_guess_draw_init,
    [DOWSING_GIVE_ITEM]     = match_substate_2,
    [DOWSING_REPLACE_ITEM]  = replace_item_draw_init,
    [DOWSING_QUITTING]      = match_substate_2,
    [DOWSING_AWAIT_INPUT]   = pw_empty_event,
    [DOWSING_REVEAL_ITEM]   = match_substate_2,
};

draw_func_t* const draw_update_funcs[N_DOWSING_STATES] = {
    [DOWSING_ENTRY]         = pw_empty_event,
    [DOWSING_CHOOSING]      = choosing_draw_update,
    [DOWSING_SELECTED]      = selected_draw_update,
    [DOWSING_INTERMEDIATE]  = pw_empty_event,
    [DOWSING_CHECK_GUESS]   = pw_empty_event,
    [DOWSING_GIVE_ITEM]     = pw_empty_event,
    [DOWSING_REPLACE_ITEM]  = replace_item_draw_update,
    [DOWSING_QUITTING]      = pw_empty_event,
    [DOWSING_AWAIT_INPUT]   = pw_empty_event,
    [DOWSING_REVEAL_ITEM]   = pw_empty_event,

};

static void switch_substate(state_vars_t *sv, uint8_t new) {
    sv->substate_2 = sv->current_substate;
    sv->current_substate = new;
}

static void move_cursor(state_vars_t *sv, int8_t m) {
    sv->cursor_2 = sv->current_cursor;
    sv->current_cursor += m;

    if(sv->current_cursor > 5) sv->current_cursor = 5;
    else if(sv->current_cursor < 0) sv->current_cursor = 0;
    pw_request_redraw();
}

static uint16_t get_item(state_vars_t *sv, route_info_t *ri, health_data_t *hd) {
    uint32_t today_steps = hd->today_steps;

    // TODO: checks for gift item
    struct {
        uint16_t le_item;
        uint16_t le_steps;
        uint8_t  percent;
    } event_item;
    pw_eeprom_read(PW_EEPROM_ADDR_SPECIAL_ITEM, (uint8_t*)(&event_item), sizeof(event_item));

    uint8_t rnd = pw_rand()%100;

    if(event_item.le_item != 0 || event_item.le_item != 0xffff) {
        if(today_steps >= event_item.le_steps) {
            if(rnd < event_item.percent) {
                return event_item.le_item;
            }
        }
    }

    for(uint8_t i = 0; i < 10; i++) {
        if(today_steps >= ri->le_route_item_steps[i]) {
            if(rnd < ri->route_item_percent[i])
                return ri->le_route_items[i];
        }
    }

    // should not get here, but just in case
    return ri->le_route_items[9];
}

static void replace_item_draw_init(state_vars_t *sv) {
    pw_screen_clear();
    pw_screen_draw_from_eeprom(
        0, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RETURN,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RETURN
    );

    pw_screen_draw_from_eeprom(
        8, 0,
        80, 16,
        PW_EEPROM_ADDR_TEXT_SWITCH,
        PW_EEPROM_SIZE_TEXT_SWITCH
    );

    for(uint8_t i = 0; i < 3; i++) {
        pw_screen_draw_from_eeprom(
            20+i*(16+8), SCREEN_HEIGHT-32-8,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM,
            PW_EEPROM_SIZE_IMG_ITEM
        );
    }
    sv->substate_2 = sv->current_substate;
}


void pw_dowsing_init(state_vars_t *sv) {
    route_info_t ri;
    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)(&ri), sizeof(ri));

    sv->reg_x = get_item(sv, &ri, &health_data_cache);

    //sv->reg_a = 0; // choose position
    sv->reg_a = pw_rand()%6;

    sv->reg_b = 0; // set no guesses
    sv->reg_c = 2; // set tries left
    sv->reg_d = 0;

    sv->current_cursor = sv->cursor_2 = 0;
    sv->current_substate = sv->substate_2 = DOWSING_ENTRY;

}

void pw_dowsing_init_display(state_vars_t *sv) {
    pw_img_t grass = {.data=img_buf, .width=16, .height=16, .size=PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK};
    pw_eeprom_read(
        PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
        grass.data,
        PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
    );

    for(uint8_t i = 0; i < 6; i++) {
        pw_screen_draw_img(&grass, 16*i, BUSH_HEIGHT);
        if(i == sv->current_cursor) {
            if(sv->anim_frame&ANIM_FRAME_NORMAL_TIME) {
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
    pw_screen_draw_text_box(0, SCREEN_HEIGHT-16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);

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

static void choosing_draw_init(state_vars_t *sv) {
    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-16,
        96, 16,
        PW_EEPROM_ADDR_TEXT_DISCOVER_ITEM,
        PW_EEPROM_SIZE_TEXT_DISCOVER_ITEM
    );
    pw_screen_draw_text_box(0, SCREEN_HEIGHT-16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);
    sv->substate_2 = sv->current_substate;

}

static void choosing_draw_update(state_vars_t *sv) {
    pw_screen_clear_area(0, SCREEN_HEIGHT-16-8, SCREEN_WIDTH, 8);
    uint16_t addr = (sv->anim_frame&ANIM_FRAME_NORMAL_TIME)?PW_EEPROM_ADDR_IMG_ARROW_UP_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_UP_OFFSET;
    pw_screen_draw_from_eeprom(
        16*sv->current_cursor+4, SCREEN_HEIGHT-16-8,
        8, 8,
        addr,
        PW_EEPROM_SIZE_IMG_ARROW
    );

}

static void selected_draw_update(state_vars_t *sv) {
    sv->reg_d++;
    uint8_t y = (sv->anim_frame&ANIM_FRAME_NORMAL_TIME)?BUSH_HEIGHT+2:BUSH_HEIGHT-2;
    pw_screen_draw_from_eeprom(
        16*sv->current_cursor, y,
        16, 16,
        PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK,
        PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK
    );
    y = (sv->anim_frame&ANIM_FRAME_NORMAL_TIME)?BUSH_HEIGHT:BUSH_HEIGHT+16-2;
    pw_screen_clear_area(16*sv->current_cursor, y, 16, 2);
}

static void replace_item_draw_update(state_vars_t *sv) {
    for(uint8_t i = 0; i < 3; i++) {
        pw_screen_clear_area(20+i*(8+16), SCREEN_HEIGHT-32, 8, 8);
    }
    if((sv->anim_frame&ANIM_FRAME_NORMAL_TIME)) {
        pw_screen_draw_from_eeprom(
            20+sv->current_cursor*(8+16), SCREEN_HEIGHT-32,
            8, 8,
            PW_EEPROM_ADDR_IMG_ARROW_UP_NORMAL,
            PW_EEPROM_SIZE_IMG_ARROW
        );
    } else {
        pw_screen_draw_from_eeprom(
            20+sv->current_cursor*(8+16), SCREEN_HEIGHT-32,
            8, 8,
            PW_EEPROM_ADDR_IMG_ARROW_UP_OFFSET,
            PW_EEPROM_SIZE_IMG_ARROW
        );
    }


    struct {
        uint16_t le_item;
        uint16_t pad;
    } inv[3];

    pw_eeprom_read(
        PW_EEPROM_ADDR_OBTAINED_ITEMS,
        (uint8_t*)inv,
        PW_EEPROM_SIZE_OBTAINED_ITEMS
    );

    uint16_t le_item = inv[sv->current_cursor].le_item;

    uint8_t idx = 0;

    struct {
        uint16_t le_item;
    } items[10];

    pw_eeprom_read(
        PW_EEPROM_ADDR_ROUTE_INFO+0x8c,
        (uint8_t*)(items),
        sizeof(items)
    );

    for(idx = 0; idx < 10; idx++) {
        if(items[idx].le_item == le_item) break;
    }

    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-16,
        96, 16,
        PW_EEPROM_ADDR_TEXT_ITEM_NAMES + idx*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE,
        PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE
    );
    pw_screen_draw_text_box(0, SCREEN_HEIGHT-16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);

}

static void check_guess_draw_init(state_vars_t *sv) {
    pw_screen_clear_area(16*sv->current_cursor, BUSH_HEIGHT, 16, 2);
    pw_screen_clear_area(16*sv->current_cursor, BUSH_HEIGHT+16-2, 16, 2);
    pw_screen_draw_from_eeprom(
        16*sv->current_cursor, BUSH_HEIGHT,
        16, 16,
        PW_EEPROM_ADDR_IMG_DOWSING_BUSH_LIGHT,
        PW_EEPROM_SIZE_IMG_DOWSING_BUSH_LIGHT
    );

    pw_screen_draw_from_eeprom(
        76, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_DIGITS + PW_EEPROM_SIZE_IMG_CHAR*sv->reg_c,
        PW_EEPROM_SIZE_IMG_CHAR
    );
    sv->substate_2 = sv->current_substate;

}

void pw_dowsing_update_display(state_vars_t *sv) {
    if(sv->substate_2 != sv->current_substate) {
        draw_init_funcs[sv->current_substate](sv);
    } else {
        draw_update_funcs[sv->current_substate](sv);
    }
}



void pw_dowsing_handle_input(state_vars_t *sv, uint8_t b) {
    switch(sv->current_substate) {
    case DOWSING_CHOOSING: {
        switch(b) {
        case BUTTON_L:
            move_cursor(sv, -1);
            break;
        case BUTTON_R:
            move_cursor(sv, +1);
            break;
        case BUTTON_M: {
            // If we haven't alreadt selected it
            if(!( (1<<sv->current_cursor) & sv->reg_b )) {
                pw_request_redraw();
                switch_substate(sv, DOWSING_SELECTED);
                break;

            }
        }
        }
        break;
    }
    case DOWSING_REPLACE_ITEM: {
        switch(b) {
        case BUTTON_R: {
            sv->current_cursor++;
            if(sv->current_cursor > 2)
                sv->current_cursor = 2;
            pw_request_redraw();
            break;
        }
        case BUTTON_L: {
            sv->current_cursor--;
            if(sv->current_cursor < 0)
                switch_substate(sv, DOWSING_QUITTING);
            pw_request_redraw();
            break;
        }
        case BUTTON_M: {
            struct {
                uint16_t le_item;
                uint16_t pad;
            } inv[3];

            pw_eeprom_read(
                PW_EEPROM_ADDR_OBTAINED_ITEMS,
                (uint8_t*)inv,
                PW_EEPROM_SIZE_OBTAINED_ITEMS
            );

            inv[sv->current_cursor].le_item = sv->reg_x;
            pw_eeprom_write(
                PW_EEPROM_ADDR_OBTAINED_ITEMS,
                (uint8_t*)inv,
                PW_EEPROM_SIZE_OBTAINED_ITEMS
            );

            switch_substate(sv, DOWSING_QUITTING);
            pw_request_redraw();
            break;
        }
        }
        break;
    }

    case DOWSING_AWAIT_INPUT: {
        sv->reg_d = 1; // we have pressed a button
        break;
    }
    default:
        break;
    }
}

void pw_dowsing_event_loop(state_vars_t *sv) {

    switch(sv->current_substate) {
    case DOWSING_ENTRY:
        switch_substate(sv, DOWSING_CHOOSING);
        break;
    case DOWSING_AWAIT_INPUT: {
        if(sv->reg_d > 0) {
            pw_request_redraw();
            switch_substate(sv, sv->substate_2);
        }
        break;
    }
    case DOWSING_SELECTED: {
        // after 4 frames, set substate check correct
        if(sv->reg_d >= 6) {
            sv->reg_d = 0;  // shakes done
            pw_request_redraw();
            switch_substate(sv, DOWSING_CHECK_GUESS);
        }
        break;
    }
    case DOWSING_CHECK_GUESS: {
        sv->reg_b |= 1<<(sv->current_cursor);   // add guess to guesses
        sv->reg_c--;

        check_guess_draw_init(sv);

        uint8_t item_pos = 1<<(sv->reg_a);
        if(item_pos & sv->reg_b) {
            sv->reg_c = 1;  // we got it right
            switch_substate(sv, DOWSING_REVEAL_ITEM);
        } else {

            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                96, 16,
                PW_EEPROM_ADDR_TEXT_NOTHING_FOUND,
                PW_EEPROM_SIZE_TEXT_NOTHING_FOUND
            );
            pw_screen_draw_text_box(0, SCREEN_HEIGHT-16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);

            // do we still have guesses remaining?
            if(sv->reg_c > 0) {
                sv->reg_d = 0;
                sv->current_substate = DOWSING_AWAIT_INPUT;
                sv->substate_2 = DOWSING_INTERMEDIATE;
            } else {
                sv->reg_c = 0;  // we got it wrong
                switch_substate(sv, DOWSING_REVEAL_ITEM);
            }

        }

        break;
    }
    case DOWSING_INTERMEDIATE: {
        if(sv->current_cursor == sv->reg_a-1 || sv->current_cursor == sv->reg_a+1) {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                96, 16,
                PW_EEPROM_ADDR_TEXT_ITS_NEAR,
                PW_EEPROM_SIZE_TEXT_ITS_NEAR
            );
            pw_screen_draw_text_box(0, SCREEN_HEIGHT-16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);
        } else {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                96, 16,
                PW_EEPROM_ADDR_TEXT_FAR_AWAY,
                PW_EEPROM_SIZE_TEXT_FAR_AWAY
            );
            pw_screen_draw_text_box(0, SCREEN_HEIGHT-16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);
        }

        sv->reg_d = 0;
        sv->current_substate = DOWSING_AWAIT_INPUT;
        sv->substate_2 = DOWSING_CHOOSING;
        break;
    }
    case DOWSING_GIVE_ITEM: {
        struct {
            uint16_t le_item;
            uint16_t pad;
        } inv[3];

        pw_eeprom_read(
            PW_EEPROM_ADDR_OBTAINED_ITEMS,
            (uint8_t*)inv,
            PW_EEPROM_SIZE_OBTAINED_ITEMS
        );

        uint8_t avail = 0;
        for(avail = 0; (avail<3) && (inv[avail].le_item != 0); avail++);

        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-16,
            96, 16,
            PW_EEPROM_ADDR_TEXT_FOUND,
            PW_EEPROM_SIZE_TEXT_FOUND
        );

        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-32,
            96, 16,
            PW_EEPROM_ADDR_TEXT_ITEM_NAMES + PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE*sv->reg_y,
            PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE
        );
        pw_screen_draw_text_box(0, SCREEN_HEIGHT-32, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, 0x3);

        if( avail >= 3 ) {
            sv->current_cursor = 0;

            sv->reg_d = 0;
            sv->current_substate = DOWSING_AWAIT_INPUT;
            sv->substate_2 = DOWSING_REPLACE_ITEM;
        } else {

            inv[avail].le_item = sv->reg_x;
            pw_eeprom_write(
                PW_EEPROM_ADDR_OBTAINED_ITEMS,
                (uint8_t*)inv,
                PW_EEPROM_SIZE_OBTAINED_ITEMS
            );

            sv->reg_d = 0;
            sv->current_substate = DOWSING_AWAIT_INPUT;
            sv->substate_2 = DOWSING_QUITTING;
        }
        break;
    }
    case DOWSING_REVEAL_ITEM: {
        pw_screen_clear_area(16*sv->reg_a, BUSH_HEIGHT-4, 16, 8);
        pw_screen_draw_from_eeprom(
            16*sv->reg_a+4, BUSH_HEIGHT,
            8, 8,
            PW_EEPROM_ADDR_IMG_ITEM,
            PW_EEPROM_SIZE_IMG_ITEM
        );

        if(sv->reg_c > 0) {
            switch_substate(sv, DOWSING_GIVE_ITEM);
        } else {
            sv->reg_d = 0;
            sv->current_substate = DOWSING_AWAIT_INPUT;
            sv->substate_2 = DOWSING_QUITTING;
        }
        break;
    }
    case DOWSING_QUITTING: {
        sv->current_cursor = 1; // back on dowsing
        pw_set_state(STATE_MAIN_MENU);
        break;
    }
    default:
        break;
    }

}

