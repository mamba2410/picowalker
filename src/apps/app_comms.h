#ifndef PW_APP_COMMS_H
#define PW_APP_COMMS_H

#include <stdint.h>
#include <stddef.h>

void pw_comms_init();
void pw_comms_event_loop();
void pw_comms_init_display();
void pw_comms_handle_input(uint8_t b);
void pw_comms_draw_update();


#endif /* PW_APP_COMMS_H */
