#ifndef PW_APP_INVENTORY_H
#define PW_APP_INVENTORY_H

#include <stdint.h>

void pw_inventory_init();
void pw_inventory_init_display();
void pw_inventory_update_display();
void pw_inventory_handle_input(uint8_t b);
//void pw_inventory_event_loop();


#endif /* PW_APP_INVENTORY_H */
