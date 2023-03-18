#ifndef PW_APP_SPLASH_H
#define PW_APP_SPLASH_H
#include "../states.h"
#include <stdint.h>

void pw_splash_init(state_vars_t *sv);
void pw_splash_handle_input(state_vars_t *sv, uint8_t b);
void pw_splash_init_display(state_vars_t *sv);
void pw_splash_update_display(state_vars_t *sv);

#endif /* PW_APP_SPLASH_H */
