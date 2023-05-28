#include <stdint.h>

#include "app_trainer_card.h"

#include "../states.h"
#include "../buttons.h"
#include "../screen.h"
#include "../utils.h"
#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../types.h"
#include "../globals.h"

/// @file app_trainer_card.c

static uint32_t prev_step_counts[7] = {0,};

void pw_trainer_card_move_cursor(state_vars_t *sv, int8_t m);

void pw_trainer_card_init(state_vars_t *sv) {
    sv->current_cursor = 0;
    sv->cursor_2 = -1;
}

void pw_trainer_card_init_display(state_vars_t *sv) {
    pw_screen_draw_from_eeprom(
        8, 0,
        80, 16,
        PW_EEPROM_ADDR_IMG_MENU_TITLE_TRAINER_CARD,
        PW_EEPROM_SIZE_IMG_MENU_TITLE_TRAINER_CARD
    );
    pw_screen_draw_from_eeprom(
        0, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RETURN,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RETURN
    );
    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-8, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RIGHT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RIGHT
    );

    pw_screen_draw_from_eeprom(
        0, 16,
        16, 16,
        PW_EEPROM_ADDR_IMG_PERSON,
        PW_EEPROM_SIZE_IMG_PERSON
    );
    pw_screen_draw_from_eeprom(
        16, 16,
        80, 16,
        PW_EEPROM_ADDR_IMG_TRAINER_NAME,
        PW_EEPROM_SIZE_IMG_TRAINER_NAME
    );
    pw_screen_draw_from_eeprom(
        0, 32,
        16, 16,
        PW_EEPROM_ADDR_IMG_ROUTE_SMALL,
        PW_EEPROM_SIZE_IMG_ROUTE_SMALL
    );
    pw_screen_draw_from_eeprom(
        16, 32,
        80, 16,
        PW_EEPROM_ADDR_TEXT_ROUTE_NAME,
        PW_EEPROM_SIZE_TEXT_ROUTE_NAME
    );
    pw_screen_draw_from_eeprom(
        0, 48,
        32, 16,
        PW_EEPROM_ADDR_IMG_TIME_FRAME,
        PW_EEPROM_SIZE_IMG_TIME_FRAME
    );
    pw_screen_draw_time(23, 59, 59, 32, 48);
}

void pw_trainer_card_draw_dayview(uint8_t day, uint32_t day_steps,
                                  uint32_t total_steps, uint16_t total_days) {

    uint8_t x=0, y=0;
    pw_screen_draw_from_eeprom(
        x, y,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_LEFT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_LEFT
    );
    x+=8;

    pw_screen_clear_area(x, y, 12, 16);
    x+=12;

    pw_screen_draw_from_eeprom(
        x, y,
        8, 16,
        PW_EEPROM_ADDR_IMG_CHAR_DASH,
        PW_EEPROM_SIZE_IMG_CHAR
    );
    x+=8;

    x+=8;
    pw_screen_draw_integer(day, x, y);

    pw_screen_draw_from_eeprom(
        x, y,
        40, 16,
        PW_EEPROM_ADDR_IMG_DAYS_FRAME,
        PW_EEPROM_SIZE_IMG_DAYS_FRAME
    );
    x+=40;

    pw_screen_clear_area(x, y, 12, 16);
    x+=12;

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-8, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_MENU_ARROW_RIGHT,
        PW_EEPROM_SIZE_IMG_MENU_ARROW_RIGHT
    );
    x+=8;

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-40, 16,
        40, 16,
        PW_EEPROM_ADDR_IMG_STEPS_FRAME,
        PW_EEPROM_SIZE_IMG_STEPS_FRAME
    );
    pw_screen_clear_area(0, 16, SCREEN_WIDTH-40, 16);
    pw_screen_draw_integer(day_steps, SCREEN_WIDTH-40, 16);

    pw_screen_draw_from_eeprom(
        0, 32,
        64, 16,
        PW_EEPROM_ADDR_IMG_TOTAL_DAYS_FRAME,
        PW_EEPROM_SIZE_IMG_TOTAL_DAYS_FRAME
    );
    pw_screen_clear_area(64, 32, SCREEN_WIDTH-64, 16);
    pw_screen_draw_integer(total_days, SCREEN_WIDTH, 32); // shift x by -1?

    pw_screen_draw_from_eeprom(
        SCREEN_WIDTH-40, 48,
        40, 16,
        PW_EEPROM_ADDR_IMG_STEPS_FRAME,
        PW_EEPROM_SIZE_IMG_STEPS_FRAME
    );
    pw_screen_clear_area(0, 48, SCREEN_WIDTH-40, 16);
    pw_screen_draw_integer(total_steps, SCREEN_WIDTH-40, 48);

}

void pw_trainer_card_move_cursor(state_vars_t *sv, int8_t m) {
    sv->current_cursor += m;
    if(sv->current_cursor < 0) sv->current_cursor = 0;
    if(sv->current_cursor > TRAINER_CARD_MAX_DAYS) sv->current_cursor = TRAINER_CARD_MAX_DAYS;
    pw_request_redraw();
}

void pw_trainer_card_handle_input(state_vars_t *sv, uint8_t b) {
    switch(b) {
    case BUTTON_L:
        if(sv->current_cursor <= 0) {
            pw_request_state(STATE_MAIN_MENU);
        } else {
            pw_trainer_card_move_cursor(sv, -1);
        }
        break;
    case BUTTON_M:
        pw_request_state(STATE_SPLASH);
        break;
    case BUTTON_R:
        pw_trainer_card_move_cursor(sv, +1);
        break;
    default:
        pw_request_state(STATE_ERROR);
        break;
    }
}

void pw_trainer_card_draw_update(state_vars_t *sv) {
    if(sv->cursor_2 != sv->current_cursor) {
        if(sv->current_cursor <= 0) {
            pw_trainer_card_init_display(sv);
        } else {
            uint32_t const total_steps = health_data_cache.total_steps;
            uint32_t const today_steps = health_data_cache.today_steps;
            uint16_t const total_days  = health_data_cache.total_days;
            pw_trainer_card_draw_dayview(
                sv->current_cursor,
                swap_bytes_u32(prev_step_counts[sv->current_cursor-1]),
                total_steps,
                total_days
            );
        }
        sv->cursor_2 = sv->current_cursor;
    }
}


