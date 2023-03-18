#ifndef PW_APP_POKE_RADAR_H
#define PW_APP_POKE_RADAR_H

#include <stdint.h>

#include "../states.h"
#include "../types.h"

void pw_poke_radar_init(state_vars_t *sv);
void pw_poke_radar_init_display(state_vars_t *sv);
void pw_poke_radar_update_display(state_vars_t *sv);
void pw_poke_radar_handle_input(state_vars_t *sv, uint8_t b);


#endif /* PW_APP_POKE_RADAR_H */
