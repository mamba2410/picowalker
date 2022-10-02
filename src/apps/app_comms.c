#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../states.h"
#include "../buttons.h"
#include "../screen.h"
#include "../pw_images.h"
#include "../ir_comms.h"
#include "app_comms.h"

static comm_state_t g_comm_state = COMM_STATE_IDLE;


void pw_comms_init() {

    g_comm_state = COMM_STATE_IDLE;
    pw_ir_set_connect_status(CONNECT_STATUS_AWAITING);

}

void pw_comms_event_loop() {

    connect_status_t cs = pw_ir_get_connect_status();
    if(cs == CONNECT_STATUS_DISCONNECTED) return;
    ir_err_t err;

    //static size_t advertising_counts = 0;

    switch(cs) {
        case CONNECT_STATUS_AWAITING:
            err = pw_ir_listen_for_handshake();
            break;
        case CONNECT_STATUS_SLAVE:
        case CONNECT_STATUS_MASTER:
            break;
        case CONNECT_STATUS_DISCONNECTED:
            break;
    }

    if(err != IR_OK) {
        return;
    }

}

void pw_comms_init_display() {

    pw_screen_draw_img(&img_pokewalker_large, (SCREEN_WIDTH-32)/2, SCREEN_HEIGHT-32-16);
    pw_screen_draw_img(&icon_connect_arcs, (SCREEN_WIDTH-8)/2, 0);
    pw_screen_draw_img(&text_connecting, 0, SCREEN_HEIGHT-16);

}

void pw_comms_handle_input(uint8_t b) {

    switch(b) {
        case BUTTON_M:
            if(pw_ir_get_connect_status() == CONNECT_STATUS_DISCONNECTED) {
                pw_request_state(STATE_SPLASH);
            }
            break;
        case BUTTON_L:
        case BUTTON_R:
        default:
            break;
    }

}

void pw_comms_draw_update() {

    static bool arcs = true;

    if(!arcs)
        pw_screen_draw_img(&icon_connect_arcs, (SCREEN_WIDTH-8)/2, 0);
    else
        pw_screen_clear_area((SCREEN_WIDTH-8)/2, 0, 8, 16);
    arcs = !arcs;

}


