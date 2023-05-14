#ifndef PW_APP_BATTLE_H
#define PW_APP_BATTLE_H

#include <stdint.h>
#include <stddef.h>

#include "../states.h"

/// @file app_battle.h

enum battle_state {
    BATTLE_OPENING,
    BATTLE_APPEARED,
    BATTLE_CHOOSING,
    BATTLE_OUR_ACTION,
    BATTLE_THEIR_ACTION,
    BATTLE_WE_LOST,
    BATTLE_THEY_FLED,
    BATTLE_STAREDOWN,
    BATTLE_CATCH_SETUP,
    BATTLE_THREW_BALL,
    BATTLE_CLOUD_ANIM,
    BATTLE_BALL_WOBBLE,
    BATTLE_ALMOST_HAD_IT,
    BATTLE_CATCH_STARS,
    BATTLE_POKEMON_CAUGHT,
    BATTLE_SWITCH,
    N_BATTLE_STATES,
};

void pw_battle_init(state_vars_t *sv);
void pw_battle_init_display(state_vars_t *sv);
void pw_battle_update_display(state_vars_t *sv);
void pw_battle_handle_input(state_vars_t *sv, uint8_t b);
void pw_battle_event_loop(state_vars_t *sv);

#endif /* PW_APP_BATTLE_H */
