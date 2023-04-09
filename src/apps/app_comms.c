#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>

#include "../states.h"
#include "../buttons.h"
#include "../screen.h"
#include "../eeprom_map.h"
#include "../ir/ir.h"
#include "../ir/actions.h"
#include "../globals.h"
#include "app_comms.h"

/*
 *  current_substate = comm_substate
 *  reg_b = screen_state
 *  reg_c = advertising_counter
 *
 */
void pw_comms_init(state_vars_t *sv) {

    sv->current_substate = COMM_SUBSTATE_FINDING_PEER;
    sv->reg_b = 0;  // TODO: screen states
    sv->reg_c = 0;  // advertising attempts
    pw_ir_set_comm_state(COMM_STATE_AWAITING);
}

void pw_comms_event_loop(state_vars_t *sv) {

    comm_state_t cs = pw_ir_get_comm_state();
    ir_err_t err = IR_ERR_UNHANDLED_ERROR;
    size_t n_rw;

    switch(cs) {
        case COMM_STATE_AWAITING: {
            err = pw_action_try_find_peer(sv, &packet_buf, PACKET_BUF_SIZE);
            break;
        }
        case COMM_STATE_SLAVE: {
            err = pw_ir_recv_packet(&packet_buf, PACKET_BUF_SIZE, &n_rw);
            if(err == IR_OK || err == IR_ERR_SIZE_MISMATCH) {
                err = pw_action_slave_perform_request(&packet_buf, n_rw);
            }
            break;
        }
        case COMM_STATE_MASTER: {
            if(sv->current_substate == COMM_SUBSTATE_AWAITING_SLAVE_ACK)
                sv->current_substate = COMM_SUBSTATE_START_PEER_PLAY;
            err = pw_action_peer_play(sv, &packet_buf, PACKET_BUF_SIZE);
            break;
        }
        case COMM_STATE_DISCONNECTED: {
            return;
        }
    } // switch(cs)

    if(err != IR_OK) {
        printf("\tError code: %02x: %s\n\tState: %d\n\tSubstate %d\n",
            err, PW_IR_ERR_NAMES[err],
            pw_ir_get_comm_state(),
            sv->current_substate
        );

        pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
        return;
    }

}

void pw_comms_init_display(state_vars_t *sv) {

    pw_screen_draw_from_eeprom(
        (SCREEN_WIDTH-32)/2, SCREEN_HEIGHT-32-16,
        32, 32,
        PW_EEPROM_ADDR_IMG_POKEWALKER_BIG,
        PW_EEPROM_SIZE_IMG_POKEWALKER_BIG
    );
    pw_screen_draw_from_eeprom(
        (SCREEN_WIDTH-8)/2, 0,
        8, 16,
        PW_EEPROM_ADDR_IMG_IR_ARCS,
        PW_EEPROM_SIZE_IMG_IR_ARCS
    );
    pw_screen_draw_from_eeprom(
        0, SCREEN_HEIGHT-16,
        96, 16,
        PW_EEPROM_ADDR_TEXT_CONNECTING,
        PW_EEPROM_SIZE_TEXT_CONNECTING
    );

}

void pw_comms_handle_input(state_vars_t *sv, uint8_t b) {

    switch(b) {
        case BUTTON_M:
            if(pw_ir_get_comm_state() == COMM_STATE_DISCONNECTED) {
                pw_request_state(STATE_SPLASH);
            }
            break;
        case BUTTON_L:
        case BUTTON_R:
        default:
            break;
    }

}

void pw_comms_draw_update(state_vars_t *sv) {

    if(sv->anim_frame) {
        pw_screen_draw_from_eeprom(
            (SCREEN_WIDTH-8)/2, 0,
            8, 16,
            PW_EEPROM_ADDR_IMG_IR_ARCS,
            PW_EEPROM_SIZE_IMG_IR_ARCS
        );
    } else
        pw_screen_clear_area((SCREEN_WIDTH-8)/2, 0, 8, 16);

}


