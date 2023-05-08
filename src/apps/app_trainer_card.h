#ifndef PW_APP_TRAINER_CARD_H
#define PW_APP_TRAINER_CARD_H

#include <stdint.h>

#include "../states.h"

/// @file app_states.h

#define TRAINER_CARD_MAX_DAYS   7

void pw_trainer_card_init(state_vars_t *sv);
void pw_trainer_card_init_display(state_vars_t *sv);
void pw_trainer_card_handle_input(state_vars_t *sv, uint8_t b);
void pw_trainer_card_draw_update(state_vars_t *sv);


#endif /* PW_APP_TRAINER_CARD_H */
