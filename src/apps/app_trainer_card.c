#include <stdint.h>

#include "app_trainer_card.h"

#include "../states.h"
#include "../buttons.h"
#include "../screen.h"
#include "../utils.h"
#include "../trainer_info.h"
#include "../eeprom_map.h"

static int8_t cursor = 0;
static int8_t prev_drawn = 0;

static uint32_t const *prev_step_counts = 0;

void pw_trainer_card_init() {
    cursor = 0;
    prev_drawn = -1;
}

void pw_trainer_card_init_display() {
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

    pw_screen_draw_from_eeprom(
            0, 48,
            40, 16,
            PW_EEPROM_ADDR_IMG_DAYS_FRAME,
            PW_EEPROM_SIZE_IMG_DAYS_FRAME
    );
    pw_screen_clear_area(8, 0, 8, 16);
    pw_screen_clear_area(40+30, 0, 20, 16);
    pw_screen_draw_integer(day, 30, 0);
    pw_screen_draw_from_eeprom(
            30-16, 0,
            8, 16,
            PW_EEPROM_ADDR_IMG_CHAR_DASH,
            PW_EEPROM_SIZE_IMG_CHAR
    );

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

void pw_move_tc_cursor(int8_t m) {
    cursor += m;
    if(cursor < 0) cursor = 0;
    if(cursor > TRAINER_CARD_MAX_DAYS) cursor = TRAINER_CARD_MAX_DAYS;
    pw_request_redraw();
}

void pw_trainer_card_handle_input(uint8_t b) {
    switch(b) {
        case BUTTON_L:
            if(cursor <= 0) {
                pw_request_state(STATE_MAIN_MENU);
            } else {
                pw_move_tc_cursor(-1);
            }
            break;
        case BUTTON_M:
            pw_request_state(STATE_SPLASH);
            break;
        case BUTTON_R:
            pw_move_tc_cursor(+1);
            break;
        default:
            pw_request_state(STATE_ERROR);
            break;
    }
}

void pw_trainer_card_draw_update() {
    if(prev_drawn != cursor) {
        if(cursor <= 0) {
            pw_trainer_card_init_display();
        } else {
            uint32_t const total_steps = swap_bytes_u32(g_reliable_data_1->health_data.be_total_steps);
            uint32_t const today_steps = swap_bytes_u32(g_reliable_data_1->health_data.be_today_steps);
            uint16_t const total_days = swap_bytes_u16(g_reliable_data_1->health_data.be_total_days);
            pw_trainer_card_draw_dayview(
                    cursor,
                    swap_bytes_u32(prev_step_counts[cursor-1]),
                    total_steps,
                    //total_steps+today_steps,
                    total_days
            );
        }
        prev_drawn = cursor;
    }
}


