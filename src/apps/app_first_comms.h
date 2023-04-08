#ifndef PW_APP_FIRST_COMMS_H
#define PW_APP_FIRST_COMMS_H

#include <stdint.h>

enum {
    FC_SUBSTATE_WAITING,
    FC_SUBSTATE_CONNECTING,
    FC_SUBSTATE_TIMEOUT,
    FC_SUBSTATE_SUCCESS,
    N_FC_SUBSTATES,
};

void pw_first_comms_init(state_vars_t *sv);
void pw_first_comms_event_loop(state_vars_t *sv);
void pw_first_comms_init_display(state_vars_t *sv);
void pw_first_comms_handle_input(state_vars_t *sv, uint8_t b);
void pw_first_comms_draw_update(state_vars_t *sv);

#endif /* PW_APP_FIRST_COMMS_H */
