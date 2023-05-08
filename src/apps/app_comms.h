#ifndef PW_APP_COMMS_H
#define PW_APP_COMMS_H

#include <stdint.h>
#include <stddef.h>

#include "../states.h"

/// @file app_comms.h

void pw_comms_init(state_vars_t *sv);
void pw_comms_event_loop(state_vars_t *sv);
void pw_comms_init_display(state_vars_t *sv);
void pw_comms_handle_input(state_vars_t *sv, uint8_t b);
void pw_comms_draw_update(state_vars_t *sv);


#endif /* PW_APP_COMMS_H */
