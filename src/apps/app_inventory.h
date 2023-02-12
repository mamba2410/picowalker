#ifndef PW_APP_INVENTORY_H
#define PW_APP_INVENTORY_H

#include <stdint.h>

#include "../states.h"

void pw_inventory_init(state_vars_t *sv);
void pw_inventory_init_display(state_vars_t *sv);
void pw_inventory_update_display(state_vars_t *sv);
void pw_inventory_handle_input(state_vars_t *sv, uint8_t b);
//void pw_inventory_event_loop();


#endif /* PW_APP_INVENTORY_H */
