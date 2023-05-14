#include <stdint.h>
#include <stddef.h>

#include "app_battle.h"
#include "../states.h"
#include "../screen.h"
#include "../eeprom_map.h"
#include "../globals.h"
#include "../buttons.h"
#include "../rand.h"

/** @file apps/app_battle.c
 *
 * ```
 *  reg_a = chosen_pokemon (0..3)
 *  reg_b = [0]=?, [1..2]=our_action, [3..4]=their_action, [5..7]=?
 *  reg_c = anim_frame
 *  reg_d = [0..3]=our_hp, [4..7]=their_hp
 *  reg_x = substate_queue index + 1
 *  reg_y = substate_queue_len
 * ```
 *
 */

#define OUR_HP_OFFSET       0
#define THEIR_HP_OFFSET     4
#define OUR_HP_MASK         (0xf<<OUR_HP_OFFSET)
#define THEIR_HP_MASK       (0xf<<THEIR_HP_OFFSET)
#define OUR_ACTION_OFFSET   1
#define OUR_ACTION_MASK     (0x3<<OUR_ACTION_OFFSET)
#define THEIR_ACTION_OFFSET 3
#define THEIR_ACTION_MASK   (0x3<<THEIR_ACTION_OFFSET)
#define CHOICE_INDEX_OFFSET 5
#define CHOICE_INDEX_MASK   (0x7<<CHOICE_INDEX_OFFSET)

#define CURRENT_WOBBLE_MASK (0x0f)
#define MAX_WOBBLE_OFFSET   4

#define ATTACK_ANIM_LENGTH 9
#define STAREDOWN_ANIM_LENGTH 5
#define THREW_BALL_ANIM_LENGTH 7
#define CLOUD_ANIM_LENGTH 2
#define WOBBLE_ANIM_LENGTH 5
#define MESSAGE_DISPLAY_ANIM_LENGTH 4
#define CATCH_ANIM_LENGTH 4

enum {
    ACTION_ATTACK,
    ACTION_EVADE,
    ACTION_SPECIAL,
    N_ACTIONS,
};

static uint8_t substate_queue[8];

// what happens to `cur` hp given both actions
// valid for both us vs. them and them vs. us
static const uint8_t HP_MATRIX[3][3] = {
    //                foe attack, foe evade,  foe crit
    /* cur attack */ {         1,         1,         2},
    /* cur evade  */ {         0,         0,         0},
    /* cur crit   */ {         1,         1,         2},
};

/*
 *  Note: same animations for attack and evade
 *  just flip ours/theirs on evade
 */
const screen_pos_t OUR_ATTACK_XS[2][ATTACK_ANIM_LENGTH] = {
    /* us   */ {56, 56, 54, 52, 53, 54, 55, 56, 56}, // copied from walker FLASH:0xbb12
    /* them */ {8,   8,  8,  8,  0,  0,  4,  8,  8}
};
const screen_pos_t THEIR_ATTACK_XS[2][ATTACK_ANIM_LENGTH] = {
    /* us   */ {56, 56, 56, 56, 64, 64, 64, 60, 56}, // copied from walker FLASH:0xbb12
    /* them */ {8,   8, 10, 12, 12, 11, 10,  9,  8}
};

#define WOBBLE_INITIAL_X 16
#define WOBBLE_INITIAL_Y 16

#define OUR_NORMAL_X    56
#define OUR_NORMAL_Y    8
#define THEIR_NORMAL_X  8
#define THEIR_NORMAL_Y  0

const uint8_t ACTION_CHANCES[5][3] = {
    // atk, evade, crit/flee
    {   45,    35,        20},
    {   40,    30,        30},
    {   50,    40,        10},
    {   60,    30,        10},
    {   20,    30,        50},
};

void pw_battle_switch_substate(state_vars_t *sv, uint8_t s) {
    sv->substate_2 = sv->current_substate;
    sv->current_substate = s;
    pw_request_redraw();
}

/**
 *  Initialises state vars for battle
 */
void pw_battle_init(state_vars_t *sv) {
    sv->current_substate = BATTLE_OPENING;
    sv->substate_2 = BATTLE_OPENING;
    sv->reg_b = 0;
    sv->reg_c = 4;
    sv->reg_d = (4<<OUR_HP_OFFSET) | (4<<THEIR_HP_OFFSET);
    printf("Pokemon index: %d\n", sv->reg_a);
}

void pw_battle_event_loop(state_vars_t *sv) {
    switch(sv->current_substate) {
    case BATTLE_OPENING: {
        if(sv->reg_c == 0) {
            pw_battle_switch_substate(sv, BATTLE_APPEARED);
        }
        break;
    }
    case BATTLE_APPEARED: {
        break;
    }
    case BATTLE_CHOOSING: {
        if(sv->current_substate != sv->substate_2) sv->reg_x = 0;
        if(sv->reg_x == 1) {
            uint8_t our_action = (sv->reg_b&OUR_ACTION_MASK)>>OUR_ACTION_OFFSET;
            uint8_t their_action = (sv->reg_b&THEIR_ACTION_MASK)>>THEIR_ACTION_OFFSET;
            uint8_t choice_index = (sv->reg_b&CHOICE_INDEX_MASK)>>CHOICE_INDEX_OFFSET;

            // decide their action
            uint8_t rnd = pw_rand()%100;
            if(rnd < ACTION_CHANCES[choice_index][2]) {
                their_action = ACTION_SPECIAL;
            } else if(rnd < (ACTION_CHANCES[choice_index][2]+ACTION_CHANCES[choice_index][1])) {
                their_action = ACTION_EVADE;
            } else {
                their_action = ACTION_ATTACK;
            }

            sv->reg_b &= ~THEIR_ACTION_MASK;
            sv->reg_b |= (their_action<<THEIR_ACTION_OFFSET)&THEIR_ACTION_MASK;

            uint8_t our_hp = (sv->reg_d&OUR_HP_MASK)>>OUR_HP_OFFSET;
            uint8_t their_hp = (sv->reg_d&THEIR_HP_MASK)>>THEIR_HP_OFFSET;

            /*
             * big matrix on what happens based on both actions
             * we can't "crit" since user input is only attack/evade.
             * so they choose to "crit" for us.
             * if we evade and they "crit" then they flee
             * if we attack and they "crit" then we have a crit hit
             *
             */
            if(our_action == ACTION_EVADE) {
                switch(their_action) {
                case ACTION_ATTACK: {
                    their_hp -= 1;
                    substate_queue[0] = BATTLE_THEIR_ACTION;
                    substate_queue[1] = BATTLE_OUR_ACTION;
                    substate_queue[2] = BATTLE_CHOOSING;
                    break;
                }
                case ACTION_EVADE: {
                    substate_queue[0] = BATTLE_STAREDOWN;
                    substate_queue[1] = BATTLE_CHOOSING;
                    break;
                }
                case ACTION_SPECIAL: {
                    substate_queue[0] = BATTLE_THEY_FLED;
                    break;
                }
                }
            } else {
                substate_queue[0] = BATTLE_OUR_ACTION;
                substate_queue[1] = BATTLE_THEIR_ACTION;
                substate_queue[2] = BATTLE_CHOOSING;

                switch(their_action) {
                case ACTION_ATTACK: {
                    our_hp -= 1;
                    their_hp -= 1;
                    sv->reg_b &= ~CHOICE_INDEX_MASK;
                    sv->reg_b |= 1<<CHOICE_INDEX_OFFSET; // taken from walker
                    break;
                }
                case ACTION_EVADE: {
                    our_hp -= 1;
                    sv->reg_b &= ~CHOICE_INDEX_MASK;
                    sv->reg_b |= 3<<CHOICE_INDEX_OFFSET; // taken from walker
                    break;
                }
                case ACTION_SPECIAL: {
                    our_hp -= 1;
                    their_hp -= 2;
                    sv->reg_b &= ~CHOICE_INDEX_MASK;
                    sv->reg_b |= 2<<CHOICE_INDEX_OFFSET; // taken from walker
                    break;
                }
                }
            }

            sv->reg_d = our_hp<<OUR_HP_OFFSET | their_hp<<THEIR_HP_OFFSET;

            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);
            sv->reg_c = 0;
        }
        break;
    }
    case BATTLE_THEIR_ACTION: {
        if(sv->reg_c == ATTACK_ANIM_LENGTH) {
            uint8_t our_hp = (sv->reg_d&OUR_HP_MASK)>>OUR_HP_OFFSET;
            if(our_hp == 0 || our_hp > 4) {
                sv->reg_c = 0;
                pw_battle_switch_substate(sv, BATTLE_WE_LOST);
                return;
            }

            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);
        }
        break;
    }
    case BATTLE_OUR_ACTION: {
        if(sv->reg_c == ATTACK_ANIM_LENGTH) {
            uint8_t their_hp = (sv->reg_d&THEIR_HP_MASK)>>THEIR_HP_OFFSET;
            if(their_hp == 0 || their_hp > 4) {
                sv->reg_c = 0;
                pw_battle_switch_substate(sv, BATTLE_THEY_FLED);
                return;
            }

            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);
        }
        break;
    }
    case BATTLE_STAREDOWN: {
        if(sv->reg_c == STAREDOWN_ANIM_LENGTH) {
            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);
        }
        break;
    }
    case BATTLE_THEY_FLED: {
        if(sv->reg_c >= MESSAGE_DISPLAY_ANIM_LENGTH) {
            pw_request_state(STATE_SPLASH);
        }
        break;
    }
    case BATTLE_WE_LOST: {
        if(sv->reg_c >= MESSAGE_DISPLAY_ANIM_LENGTH) {
            pw_request_state(STATE_SPLASH);
        }
        break;
    }
    case BATTLE_CATCH_SETUP: {

        substate_queue[0] = BATTLE_THREW_BALL;
        substate_queue[1] = BATTLE_CLOUD_ANIM;

        uint8_t n_wobbles = pw_rand()%4;
        printf("max wobbles: %d\n", n_wobbles);
        sv->reg_b = n_wobbles<<MAX_WOBBLE_OFFSET;  // reuse reg_b
        substate_queue[2] = BATTLE_BALL_WOBBLE;

        // TODO: proper thing
        bool caught = pw_rand()%2;

        if(caught) {
            substate_queue[3] = BATTLE_CLOUD_ANIM;
            substate_queue[4] = BATTLE_ALMOST_HAD_IT;
            substate_queue[5] = BATTLE_THEY_FLED;
            sv->reg_y = 6;
        } else {
            substate_queue[3] = BATTLE_CATCH_STARS;
            substate_queue[4] = BATTLE_POKEMON_CAUGHT;
            sv->reg_y = 5;
        }

        sv->reg_x = 1;  // 1-indexed
        sv->reg_c = 0;  // reset anim frame count
        pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);

        break;
    }
    case BATTLE_THREW_BALL: {
        if(sv->reg_c >= THREW_BALL_ANIM_LENGTH) {
            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);
        }
        break;
    }
    case BATTLE_CLOUD_ANIM: {
        if(sv->reg_c >= CLOUD_ANIM_LENGTH) {
            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);

        }
        break;
    }
    case BATTLE_BALL_WOBBLE: {
        if(sv->reg_c >= WOBBLE_ANIM_LENGTH) {
            uint8_t current_wobble = sv->reg_b & CURRENT_WOBBLE_MASK;
            uint8_t max_wobble = sv->reg_b >> MAX_WOBBLE_OFFSET;
            printf("current wobbles: %d\n", current_wobble);

            if(current_wobble < max_wobble) {
                current_wobble++;
                sv->reg_b &= ~CURRENT_WOBBLE_MASK;
                sv->reg_b |= current_wobble;
                sv->reg_c = 0;
            } else {
                sv->reg_x++;
                sv->reg_c = 0;
                pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);

            }
        }
        break;
    }
    case BATTLE_ALMOST_HAD_IT: {
        if(sv->reg_c >= MESSAGE_DISPLAY_ANIM_LENGTH) {
            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);

        }
        break;
    }
    case BATTLE_POKEMON_CAUGHT: {
        if(sv->reg_c >= MESSAGE_DISPLAY_ANIM_LENGTH) {
            /*
             *  TODO:
             *  - check if switch screen needed
             *  - actually store pokemon caught
             */
        }
        break;
    }
    case BATTLE_CATCH_STARS: {
        if(sv->reg_c >= CATCH_ANIM_LENGTH) {
            sv->reg_x++;
            sv->reg_c = 0;
            pw_battle_switch_substate(sv, substate_queue[sv->reg_x-1]);
        }
        break;
    }
    default: {
        printf("[ERROR] Unknown substate init: %02x\n", sv->current_substate);
        break;
    }

    }
}

void pw_battle_init_display(state_vars_t *sv) {

    pw_img_t our_sprite   = {.width=32, .height=24, .size=192, .data=eeprom_buf};
    pw_img_t their_sprite = {.width=32, .height=24, .size=192, .data=decompression_buf};

    eeprom_addr_t addr = PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED +
                         PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME
                         *(2*sv->reg_a + (sv->anim_frame&1));
    pw_eeprom_read(addr, their_sprite.data, their_sprite.size);

    addr = PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED +
           (sv->anim_frame&1)*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME;
    pw_eeprom_read(addr, our_sprite.data, our_sprite.size);

    switch(sv->current_substate) {
    case BATTLE_OPENING: {
        pw_screen_fill_area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BLACK);
        break;
    }
    case BATTLE_APPEARED: {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-16,
            96, 16,
            PW_EEPROM_ADDR_TEXT_APPEARED,
            PW_EEPROM_SIZE_TEXT_APPEARED
        );
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-32,
            80, 16,
            PW_EEPROM_ADDR_TEXT_POKEMON_NAMES + sv->reg_a*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
            PW_EEPROM_SIZE_TEXT_POKEMON_NAME
        );

        pw_img_t health_bar = {.width=8, .height=8, .data=eeprom_buf, .size=16};
        pw_eeprom_read(PW_EEPROM_ADDR_IMG_RADAR_HP_BLIP, eeprom_buf, PW_EEPROM_SIZE_IMG_RADAR_HP_BLIP);

        int8_t health = (sv->reg_d&THEIR_HP_MASK) >> THEIR_HP_OFFSET;
        for(int8_t i = 0; i < health; i++) {
            pw_screen_draw_img(&health_bar, 8*(i+1), 24);
        }

        health = (sv->reg_d&OUR_HP_MASK) >> OUR_HP_OFFSET;
        for(int8_t i = 0; i < health; i++) {
            pw_screen_draw_img(&health_bar, SCREEN_WIDTH/2 + 8*(i+1), 0);
        }

        eeprom_addr_t addr = PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED +
                             PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME
                             *(2*sv->reg_a + (sv->anim_frame&1));
        pw_screen_draw_from_eeprom(
            8, 0,
            32, 24,
            addr,
            PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME
        );

        addr = PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED +
               (sv->anim_frame&1)*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME;
        pw_screen_draw_from_eeprom(
            SCREEN_WIDTH/2+8, 8,
            32, 24,
            addr,
            PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME
        );

        break;
    }
    case BATTLE_CHOOSING: {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-32,
            96, 32,
            PW_EEPROM_ADDR_TEXT_RADAR_ACTION,
            PW_EEPROM_SIZE_TEXT_RADAR_ACTION
        );
        break;
    }
    case BATTLE_OUR_ACTION: {
        uint8_t our_action = (sv->reg_b&OUR_ACTION_MASK)>>OUR_ACTION_OFFSET;
        uint8_t their_action = (sv->reg_b&THEIR_ACTION_MASK)>>THEIR_ACTION_OFFSET;

        switch(their_action) {
        case ACTION_ATTACK: {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-32,
                80, 16,
                PW_EEPROM_ADDR_TEXT_POKEMON_NAME,
                PW_EEPROM_SIZE_TEXT_POKEMON_NAME
            );
            pw_screen_clear_area(80, SCREEN_HEIGHT-32, SCREEN_WIDTH-80, 16);
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                SCREEN_WIDTH, 16,
                PW_EEPROM_ADDR_TEXT_ATTACKED,
                PW_EEPROM_SIZE_TEXT_ATTACKED
            );
            break;
        }
        case ACTION_EVADE: {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-32,
                80, 16,
                PW_EEPROM_ADDR_TEXT_POKEMON_NAMES + sv->reg_a*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
                PW_EEPROM_SIZE_TEXT_POKEMON_NAME
            );
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                SCREEN_WIDTH, 16,
                PW_EEPROM_ADDR_TEXT_EVADED,
                PW_EEPROM_SIZE_TEXT_EVADED
            );

            break;
        }
        case ACTION_SPECIAL: {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-32,
                SCREEN_WIDTH, 16,
                PW_EEPROM_ADDR_TEXT_CRITICAL_HIT,
                PW_EEPROM_SIZE_TEXT_CRITICAL_HIT
            );
            pw_screen_clear_area(0, SCREEN_HEIGHT-16, SCREEN_WIDTH, 16);
            break;
        }
        }
        break;
    }
    case BATTLE_THEIR_ACTION: {
        uint8_t our_action = (sv->reg_b&OUR_ACTION_MASK)>>OUR_ACTION_OFFSET;
        uint8_t their_action = (sv->reg_b&THEIR_ACTION_MASK)>>THEIR_ACTION_OFFSET;
        if(our_action == ACTION_EVADE) {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-32,
                80, 16,
                PW_EEPROM_ADDR_TEXT_POKEMON_NAME,
                PW_EEPROM_SIZE_TEXT_POKEMON_NAME
            );
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                SCREEN_WIDTH, 16,
                PW_EEPROM_ADDR_TEXT_EVADED,
                PW_EEPROM_SIZE_TEXT_EVADED
            );

        } else {
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-32,
                80, 16,
                PW_EEPROM_ADDR_TEXT_POKEMON_NAMES + sv->reg_a*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
                PW_EEPROM_SIZE_TEXT_POKEMON_NAME
            );
            pw_screen_clear_area(80, SCREEN_HEIGHT-32, SCREEN_WIDTH-80, 16);
            pw_screen_draw_from_eeprom(
                0, SCREEN_HEIGHT-16,
                SCREEN_WIDTH, 16,
                PW_EEPROM_ADDR_TEXT_ATTACKED,
                PW_EEPROM_SIZE_TEXT_ATTACKED
            );
        }
        break;
    }
    case BATTLE_THEY_FLED: {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-32,
            80, 16,
            PW_EEPROM_ADDR_TEXT_POKEMON_NAMES + sv->reg_a*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
            PW_EEPROM_SIZE_TEXT_POKEMON_NAME
        );
        pw_screen_draw_message(SCREEN_HEIGHT-16, 33, 16); // "fled..."
        break;
    }
    case BATTLE_WE_LOST: {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-32,
            80, 16,
            PW_EEPROM_ADDR_TEXT_POKEMON_NAMES + sv->reg_a*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
            PW_EEPROM_SIZE_TEXT_POKEMON_NAME
        );
        pw_screen_draw_message(SCREEN_HEIGHT-16, 34, 16); // "was too strong..."
        break;
    }
    case BATTLE_STAREDOWN: {
        pw_screen_clear_area(0, SCREEN_HEIGHT-32, SCREEN_WIDTH, 16);
        pw_screen_draw_message(SCREEN_HEIGHT-16, 41, 16);
        break;
    }
    case BATTLE_THREW_BALL: {
        pw_screen_clear_area(0, SCREEN_HEIGHT-32, SCREEN_WIDTH, 16);
        pw_screen_draw_message(SCREEN_HEIGHT-16, 39, 16); // "threw a ball"
        break;
    }
    case BATTLE_CLOUD_ANIM: {
        pw_screen_clear_area(0, SCREEN_HEIGHT-32, SCREEN_WIDTH, 16);
        pw_screen_draw_from_eeprom(
            THEIR_NORMAL_X, THEIR_NORMAL_Y,
            32, 24,
            PW_EEPROM_ADDR_IMG_RADAR_APPEAR_CLOUD,
            PW_EEPROM_SIZE_IMG_RADAR_APPEAR_CLOUD
        );
        break;
    }
    case BATTLE_BALL_WOBBLE: {
        pw_screen_clear_area(0, SCREEN_HEIGHT-32, SCREEN_WIDTH, 16);
        pw_screen_clear_area(
            THEIR_NORMAL_X, THEIR_NORMAL_Y,
            32, 24
        );
        pw_screen_draw_from_eeprom(
            WOBBLE_INITIAL_Y, WOBBLE_INITIAL_Y,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL,
            PW_EEPROM_SIZE_IMG_BALL
        );
        break;
    }
    case BATTLE_ALMOST_HAD_IT: {
        pw_screen_draw_img(&their_sprite, THEIR_NORMAL_X, THEIR_NORMAL_Y);
        pw_screen_draw_message(SCREEN_HEIGHT-16, 40, 16); // "almost had it"
        break;
    }
    case BATTLE_CATCH_STARS: {
        pw_screen_clear_area(0, SCREEN_HEIGHT-32, SCREEN_WIDTH, 16);
        break;
    }
    case BATTLE_POKEMON_CAUGHT: {
        pw_screen_draw_from_eeprom(
            0, SCREEN_HEIGHT-32,
            80, 16,
            PW_EEPROM_ADDR_TEXT_POKEMON_NAMES + sv->reg_a*PW_EEPROM_SIZE_TEXT_POKEMON_NAME,
            PW_EEPROM_SIZE_TEXT_POKEMON_NAME
        );
        pw_screen_draw_message(SCREEN_HEIGHT-16, 32, 16); // "was caught!"
        break;
    }
    default: {
        printf("[ERROR] Unhandled substate draw: %d\n", sv->current_substate);
        break;
    }
    }
}

/*
 * coords:
 *   - i attack, they attack: me attack + they hit ("i attacked")-> me hit + they attack ("they attacked")
 *   - i evade, they attack: me evade + they hit ("I evaded")-> me attack + they hit ("I attacked")
 *   - i attack, they evade: me attack + they evade ("they evaded")-> me hit + they attack ("they attacked")
 *   - i evade, they evade: staredown ("staredown")
 *
 *   00 - me -> them
 *   01 - me -> them
 *   10 - them -> me
 *   11 - none - none
 */
void pw_battle_update_display(state_vars_t *sv) {
    if(sv->current_substate != sv->substate_2) {
        sv->substate_2 = sv->current_substate;
        pw_battle_init_display(sv);
        return;
    }

    pw_img_t our_sprite   = {.width=32, .height=24, .size=192, .data=eeprom_buf};
    pw_img_t their_sprite = {.width=32, .height=24, .size=192, .data=decompression_buf};

    eeprom_addr_t addr = PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED +
                         PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME
                         *(2*sv->reg_a + (sv->anim_frame&1));
    pw_eeprom_read(addr, their_sprite.data, their_sprite.size);

    addr = PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED +
           (sv->anim_frame&1)*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME;
    pw_eeprom_read(addr, our_sprite.data, our_sprite.size);

    switch(sv->current_substate) {
    case BATTLE_OPENING: {
        if(sv->reg_c > 0) sv->reg_c--;
        pw_screen_fill_area(0, sv->reg_c*8, SCREEN_WIDTH, (4-sv->reg_c)*16, SCREEN_WHITE);
        break;
    }
    case BATTLE_APPEARED: {
        pw_screen_draw_img(&their_sprite, 8, 0);
        pw_screen_draw_img(&our_sprite, SCREEN_WIDTH/2+8, 8);
        break;
    }
    case BATTLE_CHOOSING: {
        pw_screen_draw_img(&their_sprite, 8, 0);
        pw_screen_draw_img(&our_sprite, SCREEN_WIDTH/2+8, 8);

        break;
    }
    case BATTLE_OUR_ACTION: {
        uint8_t our_action = (sv->reg_b&OUR_ACTION_MASK)>>OUR_ACTION_OFFSET;
        uint8_t their_action = (sv->reg_b&THEIR_ACTION_MASK)>>THEIR_ACTION_OFFSET;
        pw_screen_draw_img(&our_sprite, OUR_ATTACK_XS[0][sv->reg_c], 8);
        pw_screen_draw_img(&their_sprite, OUR_ATTACK_XS[1][sv->reg_c], 0);

        if(sv->reg_c == (ATTACK_ANIM_LENGTH+1)/2) {
            if(their_action == ACTION_SPECIAL) {
                pw_screen_draw_from_eeprom(
                    (SCREEN_WIDTH-16)/2, 0,
                    16, 32,
                    PW_EEPROM_ADDR_IMG_RADAR_CRITICAL_HIT,
                    PW_EEPROM_ADDR_IMG_RADAR_CRITICAL_HIT
                );
            } else if(their_action != ACTION_EVADE) {
                pw_screen_draw_from_eeprom(
                    (SCREEN_WIDTH-16)/2, 0,
                    16, 32,
                    PW_EEPROM_ADDR_IMG_RADAR_ATTACK_HIT,
                    PW_EEPROM_ADDR_IMG_RADAR_ATTACK_HIT
                );
            }

            uint8_t hp = (sv->reg_d&THEIR_HP_MASK)>>THEIR_HP_OFFSET;
            pw_screen_clear_area(8*(hp+1), 24, 8*(4-hp), 8);
        } else {
            pw_screen_clear_area(
                (SCREEN_WIDTH-16)/2, 0,
                16, 32
            );
        }

        sv->reg_c++;
        break;
    }
    case BATTLE_THEIR_ACTION: {
        uint8_t our_action = (sv->reg_b&OUR_ACTION_MASK)>>OUR_ACTION_OFFSET;
        uint8_t their_action = (sv->reg_b&THEIR_ACTION_MASK)>>THEIR_ACTION_OFFSET;
        pw_screen_draw_img(&our_sprite,   THEIR_ATTACK_XS[0][sv->reg_c], 8);
        pw_screen_draw_img(&their_sprite, THEIR_ATTACK_XS[1][sv->reg_c], 0);

        if(sv->reg_c == (ATTACK_ANIM_LENGTH+1)/2) {
            if(our_action != ACTION_EVADE) {
                pw_screen_draw_from_eeprom(
                    (SCREEN_WIDTH-16)/2, 0,
                    16, 32,
                    PW_EEPROM_ADDR_IMG_RADAR_ATTACK_HIT,
                    PW_EEPROM_ADDR_IMG_RADAR_ATTACK_HIT
                );
            }

            uint8_t hp = (sv->reg_d&OUR_HP_MASK)>>OUR_HP_OFFSET;
            pw_screen_clear_area(SCREEN_WIDTH/2+8*(hp+1), 0, 8*(4-hp), 8);

        } else {
            pw_screen_clear_area(
                (SCREEN_WIDTH-16)/2, 0,
                16, 32
            );
        }

        sv->reg_c++;
        break;
    }
    case BATTLE_WE_LOST: {
        break;
    }
    case BATTLE_THEY_FLED: {
        // TODO: animation
        break;
    }
    case BATTLE_STAREDOWN: {
        pw_screen_draw_img(&our_sprite,   THEIR_ATTACK_XS[0][0], 8);
        pw_screen_draw_img(&their_sprite, THEIR_ATTACK_XS[1][0], 0);
        sv->reg_c++;
        break;
    }
    case BATTLE_THREW_BALL: {
        pw_screen_draw_from_eeprom(
            OUR_ATTACK_XS[0][sv->reg_c], 16,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL,
            PW_EEPROM_SIZE_IMG_BALL
        );
        sv->reg_c++;
        break;
    }
    case BATTLE_CLOUD_ANIM: {
        sv->reg_c++;
        break;
    }
    case BATTLE_BALL_WOBBLE: {

        screen_pos_t middle = THEIR_NORMAL_X+8;
        screen_pos_t left   = middle-2;
        screen_pos_t right  = middle+2;

        screen_pos_t x = middle;
        switch(sv->reg_c%4) {
        case 0:
        case 2: {
            x=middle;
            break;
        }
        case 1: {
            x=left;
            break;
        }
        case 3: {
            x=right;
            break;
        }
        }

        printf("%d\n", x);
        pw_screen_clear_area(left, 16, right-left+8, 8);
        pw_screen_draw_from_eeprom(
            x, 16,
            8, 8,
            PW_EEPROM_ADDR_IMG_BALL,
            PW_EEPROM_SIZE_IMG_BALL
        );
        sv->reg_c++;
        break;
    }
    case BATTLE_ALMOST_HAD_IT: {
        pw_screen_draw_img(&their_sprite, THEIR_NORMAL_X, THEIR_NORMAL_Y);
        sv->reg_c++;
        break;
    }
    case BATTLE_CATCH_STARS: {
        pw_screen_draw_from_eeprom(
            THEIR_NORMAL_X, THEIR_NORMAL_Y+8-sv->reg_c,
            8, 8,
            PW_EEPROM_ADDR_IMG_RADAR_CATCH_EFFECT,
            PW_EEPROM_SIZE_IMG_RADAR_CATCH_EFFECT
        );
        sv->reg_c++;
        break;
    }
    case BATTLE_POKEMON_CAUGHT: {
        break;
    }
    default: {
        printf("[ERROR] Unhandled substate draw update: %02x\n", sv->current_substate);
        break;
    }

    }

}

void pw_battle_handle_input(state_vars_t *sv, uint8_t b) {
    switch(sv->current_substate) {
    case BATTLE_APPEARED: {
        pw_battle_switch_substate(sv, BATTLE_CHOOSING);
        break;
    }
    case BATTLE_CHOOSING: {
        switch(b) {
        case BUTTON_L: {
            sv->reg_b &= ~OUR_ACTION_MASK;
            sv->reg_b |= ACTION_ATTACK << OUR_ACTION_OFFSET;
            sv->reg_x = 1;
            break;
        }
        case BUTTON_R: {
            sv->reg_b &= ~OUR_ACTION_MASK;
            sv->reg_b |= ACTION_EVADE << OUR_ACTION_OFFSET;
            sv->reg_x = 1;
            break;
        }
        case BUTTON_M: {
            pw_battle_switch_substate(sv, BATTLE_CATCH_SETUP);
            break;
        }
        }
        break;
    }
    case BATTLE_POKEMON_CAUGHT:
    case BATTLE_THEY_FLED:
    case BATTLE_WE_LOST: {
        pw_request_state(STATE_SPLASH);
        break;
    }
    default:
        break;
    }

}



