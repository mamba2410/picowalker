#ifndef PW_MENU_H
#define PW_MENU_H

#include <stdbool.h>
#include <stdint.h>

void pw_menu_init_display();
void pw_menu_handle_input(uint8_t b);
void pw_menu_set_cursor(int8_t c);


bool move_cursor(int8_t move);

extern const int8_t MENU_SIZE;

#endif /* PW_MENU_H */
