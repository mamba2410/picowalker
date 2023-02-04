#ifndef PW_APP_DOWSING_H
#define PW_APP_DOWSING_H

#include <stdint.h>

#include "../states.h"

enum dowsing_state {
    DOWSING_ENTRY,
    DOWSING_CHOOSING,
    DOWSING_SELECTED,
    DOWSING_INTERMEDIATE,
    DOWSING_CHECK_GUESS,
    DOWSING_GIVE_ITEM,
    DOWSING_REPLACE_ITEM,
    DOWSING_QUITTING,
    DOWSING_AWAIT_INPUT,
    DOWSING_REVEAL_ITEM,
    N_DOWSING_STATES
};

void pw_dowsing_update_display(state_vars_t *sv);
void pw_dowsing_handle_input(state_vars_t *sv, uint8_t b);
void pw_dowsing_event_loop(state_vars_t *sv);
void pw_dowsing_init(state_vars_t *sv);
void pw_dowsing_init_display(state_vars_t *sv);

#endif /* PW_APP_DOWSING_H */
