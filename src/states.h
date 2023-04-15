#ifndef PW_STATES_H
#define PW_STATES_H

#include <stdbool.h>
#include <stdint.h>

#define PW_REQUEST_REDRAW       (1<<0)
#define PW_REQUEST_STATE_CHANGE (1<<1)

#define PW_CLR_REQUEST(x, y) x &= ~(y)
#define PW_SET_REQUEST(x, y) x |=  (y)
#define PW_GET_REQUEST(x, y) (x&(y))

typedef enum {
    STATE_SCREENSAVER,
    STATE_SPLASH,
    STATE_MAIN_MENU,
    STATE_POKE_RADAR,
    STATE_DOWSING,
    STATE_CONNECT,
    STATE_TRAINER_CARD,
    STATE_INVENTORY,
    STATE_SETTINGS,
    STATE_ERROR,
    STATE_FIRST_CONNECT,
    STATE_BATTLE,
    N_STATES,
} pw_state_t;

/*
 *  Struct to keep all state variables the same across states
 */
typedef struct {
    int8_t  current_cursor;
    int8_t  cursor_2;
    uint8_t current_substate;
    uint8_t substate_2;
    uint8_t anim_frame;
    uint8_t reg_a;
    uint8_t reg_b;
    uint8_t reg_c;
    uint8_t reg_d;
    uint16_t reg_x;
    uint16_t reg_y;
    uint16_t reg_z;
} state_vars_t;

typedef void (state_draw_func_t)(state_vars_t*);
typedef void (state_event_func_t)(state_vars_t*);
typedef void (state_input_func_t)(state_vars_t*, uint8_t);
typedef void (draw_func_t)(state_vars_t*);


/*
 *  Use an array of structures to represent each state.
 *  Each state should have:
 *  - name/string
 *  - initialiser function
 *  - event loop function
 *  - input handler function
 *  - draw initialiser function
 *  - draw update function
 */
extern const char* const state_strings[];
extern state_event_func_t* const state_init_funcs[];
extern state_event_func_t* const state_event_loop_funcs[];
extern state_input_func_t* const state_input_funcs[];
extern state_draw_func_t* const state_draw_init_funcs[];
extern state_draw_func_t* const state_draw_update_funcs[];

void pw_send_to_error(state_vars_t *sv, uint8_t b);
void pw_send_to_splash(state_vars_t *sv, uint8_t b);

void pw_request_redraw();
void pw_request_state(pw_state_t s);

bool pw_set_state(pw_state_t s);
pw_state_t pw_get_state();

void pw_state_init();
void pw_state_run_event_loop();
void pw_state_handle_input(uint8_t b);
void pw_state_draw_init();
void pw_state_draw_update();

/*
 *  State functions
 */
void pw_empty_event(state_vars_t *sv);
void pw_empty_input(state_vars_t *sv, uint8_t b);

// STATE_ERROR
void pw_error_init_display(state_vars_t *sv);

#endif /* PW_STATES_H */

