#ifndef PW_APP_POKE_RADAR_H
#define PW_APP_POKE_RADAR_H

#include <stdint.h>

#include "../states.h"
#include "../types.h"

/*
 *  Copying what the walker does.#
 *  ABC is 1+index in eeprom
 *  event is just 4
 *  idk why they did this, since all they ever do is subtract 1 again
 *  but hey, i'll be consistent
 */
#define OPTION_A        0
#define OPTION_B        1
#define OPTION_C        2
#define OPTION_EVENT    3

enum {
    RADAR_CHOOSING,
    RADAR_BUSH_OK,
    RADAR_FAILED,
    N_RADAR_STATES,
};

void pw_poke_radar_init(state_vars_t *sv);
void pw_poke_radar_init_display(state_vars_t *sv);
void pw_poke_radar_update_display(state_vars_t *sv);
void pw_poke_radar_handle_input(state_vars_t *sv, uint8_t b);
void pw_poke_radar_event_loop(state_vars_t *sv);

void pw_poke_radar_choose_pokemon(state_vars_t *sv, route_info_t *ri, health_data_t *hd);

#endif /* PW_APP_POKE_RADAR_H */
