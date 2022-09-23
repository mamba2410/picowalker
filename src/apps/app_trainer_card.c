#include <stdint.h>

#include "app_trainer_card.h"

#include "../states.h"
#include "../buttons.h"
#include "../screen.h"
#include "../pw_images.h"
#include "../pwroms.h"

static int8_t cursor = 0;
static int8_t prev_drawn = 0;

static uint32_t *prev_step_counts = (uint32_t*)&(eeprom[0xcef0]);

void pw_trainer_card_init() {
    cursor = 0;
    prev_drawn = -1;
}

void pw_trainer_card_init_display() {
    pw_screen_draw_img(&text_mm_trainer, 8, 0);
    pw_screen_draw_img(&icon_mm_return, 0, 0);
    pw_screen_draw_img(&icon_mm_rarrow, SCREEN_WIDTH-icon_mm_rarrow.width, 0);

    pw_screen_draw_img(&icon_generic_person, 0, 16);
    pw_screen_draw_img(&text_trainer_name, 16, 16);
    pw_screen_draw_img(&icon_small_route, 0, 32);
    pw_screen_draw_img(&text_route_name, 16, 32);
    pw_screen_draw_img(&text_time, 0, 48);
    pw_screen_draw_time(23, 59, 59, 32, 48);
}

void pw_trainer_card_draw_dayview(uint8_t day, uint32_t day_steps,
        uint32_t total_steps, uint16_t total_days) {

    pw_screen_draw_img(&icon_mm_larrow, 0, 0);
    pw_screen_draw_img(&icon_mm_rarrow, SCREEN_WIDTH-icon_mm_rarrow.width, 0);

    pw_screen_draw_img(&text_days, 30, 0);
    pw_screen_clear_area(icon_mm_larrow.width, 0, 8, 16);
    pw_screen_clear_area(text_days.width+30, 0, 20, 16);
    pw_screen_draw_integer(day, 30, 0);
    pw_screen_draw_img(&text_character_dash, 30-16, 0);

    pw_screen_draw_img(&text_steps, SCREEN_WIDTH-text_steps.width, 16);
    pw_screen_clear_area(0, 16, SCREEN_WIDTH-text_steps.width, 16);
    pw_screen_draw_integer(day_steps, SCREEN_WIDTH-text_steps.width, 16);

    pw_screen_draw_img(&text_total_days, 0, 32);
    pw_screen_clear_area(text_total_days.width, 32, SCREEN_WIDTH-text_total_days.width, 16);
    pw_screen_draw_integer(total_days, SCREEN_WIDTH-1, 32);

    pw_screen_draw_img(&text_steps, SCREEN_WIDTH-text_steps.width, SCREEN_HEIGHT-16);
    pw_screen_clear_area(0, 48, SCREEN_WIDTH-text_steps.width, 16);
    pw_screen_draw_integer(total_steps, SCREEN_WIDTH-text_steps.width, SCREEN_HEIGHT-16);

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
            pw_trainer_card_draw_dayview(cursor, prev_step_counts[cursor-1], 0, 0);
        }
        prev_drawn = cursor;
    }
}


