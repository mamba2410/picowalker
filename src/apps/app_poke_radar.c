#include <stdint.h>
#include <stddef.h>

#include "app_poke_radar.h"

#include "../states.h"
#include "../types.h"
#include "../globals.h"
#include "../screen.h"
#include "../utils.h"
#include "../eeprom.h"
#include "../eeprom_map.h"
#include "../rand.h"
#include "../buttons.h"

/** @file app_poke_radar.c
 * Radar find pokemon game
 *
 * ```
 *
 * current_cursor = user cursor
 * cursor_2 = active_bush
 * current_substate
 * substate_2 = prev_substate
 * reg_a = chosen_mon (0..3)
 * reg_b = radar_level (0..3)
 * reg_c = current_level (0..3)
 * reg_x = invisible_timer
 * reg_y = active_timer
 * ```
 *
 */

static uint8_t radar_level_to_index[4] = {0, 0, 1, 2};
static screen_pos_t bush_xs[4] = {8, 16, 56, 64};
static screen_pos_t bush_ys[4] = {0, 24, 0, 24};
static uint8_t invisible_timer_divisors[4] = {1, 1, 2, 3}; // idk
static uint8_t active_timers[4] = {20, 15, 10, 7};


static void draw_cursor_update(state_vars_t *sv) {
    pw_screen_draw_from_eeprom(
        bush_xs[sv->current_cursor]-8, bush_ys[sv->current_cursor]+8,
        8, 8,
        (sv->anim_frame&ANIM_FRAME_NORMAL_TIME)? PW_EEPROM_ADDR_IMG_ARROW_RIGHT_NORMAL:PW_EEPROM_ADDR_IMG_ARROW_RIGHT_OFFSET,
        PW_EEPROM_SIZE_IMG_ARROW
    );
    for(uint8_t i = 0; i < 4; i++) {
        if(i == sv->current_cursor) continue;
        pw_screen_clear_area(bush_xs[i]-8, bush_ys[i]+8, 8, 8);
    }
}

void pw_poke_radar_init(state_vars_t *sv) {
    route_info_t ri;
    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)&ri, sizeof(ri));

    pw_poke_radar_choose_pokemon(sv, &ri, &health_data_cache);
    sv->current_cursor = 0;
    sv->cursor_2 = pw_rand()%4;
    sv->current_substate = RADAR_CHOOSING;
    sv->substate_2 = RADAR_CHOOSING;
    sv->reg_c = 0;
    sv->reg_x = 2+(pw_rand()%10)/invisible_timer_divisors[sv->reg_c];
    sv->reg_y = active_timers[sv->reg_c];
    sv->reg_z = 0;
}

void pw_poke_radar_init_display(state_vars_t *sv) {
    switch(sv->current_substate) {
    case RADAR_CHOOSING: {
        pw_img_t bush = {.width=32, .height=24, .data=eeprom_buf, .size=192};
        pw_eeprom_read(PW_EEPROM_ADDR_IMG_RADAR_BUSH, eeprom_buf, PW_EEPROM_SIZE_IMG_RADAR_BUSH);

        for(uint8_t i = 0; i < 4; i++)
            pw_screen_draw_img(&bush, bush_xs[i], bush_ys[i]);

        pw_screen_draw_message(SCREEN_HEIGHT-16, 28, 16); // "find a pokemon!"

        break;
    }
    case RADAR_BUSH_OK: {
        pw_screen_draw_from_eeprom(
            bush_xs[sv->cursor_2]+16, bush_ys[sv->cursor_2],
            16, 16,
            PW_EEPROM_ADDR_IMG_RADAR_CLICK,
            PW_EEPROM_SIZE_IMG_RADAR_CLICK
        );
        break;
    }
    case RADAR_FAILED: {
        pw_screen_draw_message(SCREEN_HEIGHT-16, 30, 16); // "it got away"
        break;
    }
    case RADAR_START_BATTLE: {
        break;
    }
    }
}

void pw_poke_radar_update_display(state_vars_t *sv) {
    if(sv->current_substate != sv->substate_2) {
        sv->substate_2 = sv->current_substate;
        pw_poke_radar_init_display(sv);
        return;
    }


    switch(sv->current_substate) {
    case RADAR_CHOOSING: {
        draw_cursor_update(sv);

        if(sv->reg_x > 0) {
            sv->reg_x--;
            break;
        }

        pw_screen_draw_from_eeprom(
            bush_xs[sv->cursor_2]+16, bush_ys[sv->cursor_2],
            16, 16,
            PW_EEPROM_ADDR_IMG_RADAR_BUBBLE_ONE + radar_level_to_index[sv->reg_c]*PW_EEPROM_SIZE_IMG_RADAR_BUBBLE_ONE,
            PW_EEPROM_SIZE_IMG_RADAR_BUBBLE_ONE
        );

        if(sv->reg_y > 0) {
            sv->reg_y--;
            break;
        }

        break;
    }
    case RADAR_BUSH_OK: {
        draw_cursor_update(sv);
        if(sv->reg_x > 0)
            sv->reg_x--;
        break;
    }
    case RADAR_FAILED: {
        draw_cursor_update(sv);
        break;
    }
    case RADAR_START_BATTLE: {
        pw_screen_fill_area(0, sv->reg_x*8, SCREEN_WIDTH, 8, SCREEN_BLACK);
        pw_screen_fill_area(0, SCREEN_HEIGHT-(sv->reg_x+1)*8, SCREEN_WIDTH, 8, SCREEN_BLACK);
        sv->reg_x++;
        break;
    }
    }
}

void pw_poke_radar_handle_input(state_vars_t *sv, uint8_t b) {
    switch(sv->current_substate) {
    case RADAR_CHOOSING: {
        switch(b) {
        case BUTTON_L: {
            sv->current_cursor = (sv->current_cursor-1)%4;
            break;
        }
        case BUTTON_R: {
            sv->current_cursor = (sv->current_cursor+1)%4;
            break;
        }
        case BUTTON_M: {
            if(sv->current_cursor == sv->cursor_2) {
                sv->current_substate = RADAR_BUSH_OK;
                sv->reg_x = sv->reg_y = 3;
            } else {
                sv->current_substate = RADAR_FAILED;
            }
            break;
        }
        }
        pw_request_redraw();
        break;
    }
    case RADAR_BUSH_OK: {
        break;
    }
    case RADAR_FAILED: {
        pw_request_state(STATE_SPLASH);
        break;
    }
    case RADAR_START_BATTLE: {
        pw_request_state(STATE_BATTLE);
        break;
    }
    }
}

void pw_poke_radar_event_loop(state_vars_t *sv) {
    switch(sv->current_substate) {
    case RADAR_CHOOSING: {
        if(sv->reg_x == 0 && sv->reg_y == 0) {
            sv->current_substate = RADAR_FAILED;
            pw_request_redraw();
        }
        break;
    }
    case RADAR_BUSH_OK: {
        if(sv->reg_x == 0) {

            if(sv->reg_c >= sv->reg_b) {
                //  move to battle state
                sv->reg_x = 0;
                sv->current_substate = RADAR_START_BATTLE;
                break;
            }

            sv->cursor_2 = pw_rand()%4;
            sv->reg_c++;                // inc exclamation level
            sv->current_substate = RADAR_CHOOSING;
            sv->reg_x = 2+(pw_rand()%10)/invisible_timer_divisors[sv->reg_c];
            sv->reg_y = active_timers[sv->reg_c];
            pw_request_redraw();
        }
        break;
    }
    case RADAR_FAILED: {
        break;
    }
    case RADAR_START_BATTLE: {
        if(sv->reg_x == 4) {
            pw_request_state(STATE_BATTLE);
        }
        break;
    }
    }

}


/*
 * Given *initialised* route_info_t and health_data_t.
 *
 * Sets reg_a, reg_b
 */
void pw_poke_radar_choose_pokemon(state_vars_t *sv, route_info_t *ri, health_data_t *hd) {
    uint32_t today_steps = swap_bytes_u32(hd->today_steps);

    special_inventory_t inv;
    pw_eeprom_read(PW_EEPROM_ADDR_RECEIVED_BITFIELD, (uint8_t*)&inv, 1);

    uint8_t event_index;
    pw_eeprom_read(PW_EEPROM_ADDR_SPECIAL_POKEMON_EVENT_INDEX, &event_index, 1);

    //pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_POKEMON, (uint8_t*)(ri->route_pokemon), PW_EEPROM_SIZE_ROUTE_POKEMON);

    // event
    if( event_index > 0 && !inv.event_pokemon) {

        pokemon_summary_t event_pokemon;
        pw_eeprom_read(
            PW_EEPROM_ADDR_SPECIAL_POKEMON_BASIC_DATA,
            (uint8_t*)&event_pokemon,
            sizeof(event_pokemon)
        );

        struct {
            uint16_t le_steps;
            uint8_t chance;
        } special;
        pw_eeprom_read(PW_EEPROM_ADDR_SPECIAL_POKEMON_STEPS_REQUIRED, (uint8_t*)&special, sizeof(special));

        if(today_steps > special.le_steps) {
            if( pw_rand()%100 < special.chance ) {
                uint32_t rnd = pw_rand();
                sv->reg_a = OPTION_EVENT;   // found event pokemon
                sv->reg_b = 2+rnd%2;        // radar level = 2 or 3
                return;
            }
        }
    }

    // not event

    uint8_t rnd = pw_rand()%100;

    for(uint8_t i = 0; i < 3; i++) {
        if(today_steps < ri->le_route_pokemon_steps[i]) continue;
        if(rnd > ri->route_pokemon_percent[i]) continue;

        sv->reg_a = i;                  // found pokemon index
        sv->reg_b = 2-i+pw_rand()%2;    // set radar level based on rarity
        return;
    }

    // unreachable
    sv->reg_a = OPTION_C;
    sv->reg_b = pw_rand()%2;
}

