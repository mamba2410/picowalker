#ifndef PW_MENU_H
#define PW_MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "states.h"

/*
 *  Message index + 1
 *  Offset from eeprom 0x4330
 */
enum {
    MSG_NONE,
    MSG_NEED_WATTS,
    MSG_NO_POKEMON_HELD,
    MSG_NOTHING_HELD,
};

void pw_menu_init(state_vars_t *sv);
void pw_menu_init_display(state_vars_t *sv);
void pw_menu_update_display(state_vars_t *sv);
void pw_menu_handle_input(state_vars_t *sv, uint8_t b);
void pw_menu_set_cursor(state_vars_t *sv, int8_t c);


bool pw_menu_move_cursor(state_vars_t *sv, int8_t move);

extern const int8_t MENU_SIZE;

#endif /* PW_MENU_H */
