#ifndef PW_APP_TRAINER_CARD_H
#define PW_APP_TRAINER_CARD_H

#include <stdint.h>

#define TRAINER_CARD_MAX_DAYS   7

void pw_trainer_card_init();
void pw_trainer_card_init_display();
void pw_trainer_card_handle_input(uint8_t b);
void pw_trainer_card_draw_update();
void pw_moe_tc_cursor(int8_t m);


#endif /* PW_APP_TRAINER_CARD_H */
