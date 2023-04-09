#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>

#include "../flash.h"
#include "../states.h"
#include "../eeprom.h"
#include "../ir/ir.h"
#include "../ir/actions.h"
#include "../screen.h"
#include "../buttons.h"
#include "../globals.h"
#include "app_first_comms.h"


/*
 *  current_substate = comm_substate
 *  reg_a = happy/sad/neutral
 *  reg_b = prev happy/sad/neutral
 *  reg_c = advertising_attempts
 *  reg_x = timer
 */
void pw_first_comms_init(state_vars_t *sv) {
     sv->current_substate = COMM_SUBSTATE_NONE;
     sv->substate_2 = FC_SUBSTATE_WAITING;
     sv->reg_c = 0;
     sv->reg_x = 0;
     pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
}

void pw_first_comms_event_loop(state_vars_t *sv) {
    comm_state_t cs = pw_ir_get_comm_state();
    ir_err_t err = IR_ERR_GENERAL;
    size_t n_rw;

    switch(cs) {
        case COMM_STATE_AWAITING: {
            err = pw_action_try_find_peer(sv, &packet_buf, PACKET_BUF_SIZE);
            break;
        }
        case COMM_STATE_SLAVE: {
            printf("Slave waiting\n");
            err = pw_ir_recv_packet(&packet_buf, PACKET_BUF_SIZE, &n_rw);
            if(err == IR_OK || err == IR_ERR_SIZE_MISMATCH) {
                err = pw_action_slave_perform_request(&packet_buf, n_rw);
            }

            if(pw_ir_get_comm_state() == COMM_STATE_DISCONNECTED && err == IR_OK) {
                    sv->substate_2 = FC_SUBSTATE_SUCCESS;
            }
            break;
        }
        case COMM_STATE_MASTER: {
            err = IR_ERR_INVALID_MASTER; // we are not allowed to be master in this state
            break;
        }
        case COMM_STATE_DISCONNECTED: {
            err = IR_OK;
            sv->reg_c = 0;
            if(sv->substate_2 == FC_SUBSTATE_TIMEOUT && sv->reg_x == 0) {
                sv->substate_2 = FC_SUBSTATE_WAITING;
            }
            if(sv->substate_2 == FC_SUBSTATE_SUCCESS) {
                pw_request_state(STATE_SPLASH);
            }
            break;
        }
        default: {
                     printf("Error: Unexpected comm state 0x%02x\n", cs);
                 }
                                      break;
    } // switch(cs)

    if(err != IR_OK) {
        printf("\tError code: %02x: %s\n\tState: %d\n\tSubstate %d\n",
            err, PW_IR_ERR_NAMES[err],
            pw_ir_get_comm_state(),
            sv->current_substate
        );

        pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
        sv->reg_c = 0;
        sv->substate_2 = FC_SUBSTATE_TIMEOUT;
        sv->reg_x = 5;
    }
}

void pw_first_comms_init_display(state_vars_t *sv) {

    pw_img_t img = {.height=32, .width=32, .size=256, .data=eeprom_buf};
    pw_flash_read(FLASH_IMG_POKEWALKER, img.data);
    pw_screen_draw_img(&img, (SCREEN_WIDTH-32)/2, (SCREEN_HEIGHT-32)/2);

    img.width = 16;
    img.height = 8;
    img.size = 0x20;
    pw_flash_read(FLASH_IMG_FACE_NEUTRAL, img.data);
    pw_screen_draw_img(&img, (SCREEN_WIDTH-16)/2, (SCREEN_HEIGHT-8)/2);

}

void pw_first_comms_handle_input(state_vars_t *sv, uint8_t b) {

    if( b == BUTTON_M && pw_ir_get_comm_state() == COMM_STATE_DISCONNECTED ) {
        // if we are actually initialised
        if(pw_eeprom_check_for_nintendo() && walker_info_cache.flags&0x01) { // TODO: walker inited flag
        //if(false) {
             pw_request_state(STATE_SPLASH);
             return;
        }
    }

    sv->reg_b = 0;
    sv->reg_c = 0;
    sv->current_substate = COMM_SUBSTATE_FINDING_PEER;
    pw_ir_set_comm_state(COMM_STATE_AWAITING);
}

void pw_first_comms_draw_update(state_vars_t *sv) {
    // TODO: update smiley faces from FLASH
    comm_state_t cs = pw_ir_get_comm_state();

    switch(cs) {
        case COMM_STATE_DISCONNECTED: {
            switch(sv->substate_2) {
                case FC_SUBSTATE_WAITING: {
                    pw_img_t img = {.width=8, .height=8, .size=16, .data=eeprom_buf};
                    if(sv->anim_frame&1) {
                        pw_flash_read(FLASH_IMG_UP_ARROW, img.data);
                        pw_screen_draw_img(&img, (SCREEN_WIDTH-8)/2, 48);
                    } else {
                        pw_screen_clear_area((SCREEN_WIDTH-8)/2, 48, 8, 8);
                    }

                    img.width = 16;
                    img.size=32;
                    pw_flash_read(FLASH_IMG_FACE_NEUTRAL, img.data);
                    pw_screen_draw_img(&img, (SCREEN_WIDTH-16)/2, (SCREEN_HEIGHT-8)/2);

                    break;
                }
                case FC_SUBSTATE_TIMEOUT: {
                    pw_screen_clear_area((SCREEN_WIDTH-8)/2, 0, 8, 8);
                    pw_img_t face = {.width=16, .height=8, .size=32, .data=eeprom_buf};
                    pw_flash_read(FLASH_IMG_FACE_SAD, face.data);
                    pw_screen_draw_img(&face, (SCREEN_WIDTH-16)/2, (SCREEN_HEIGHT-8)/2);
                    sv->reg_x--;
                    break;
                }
            }
            break;
        }
        case COMM_STATE_AWAITING:
        case COMM_STATE_SLAVE: {
            pw_img_t face = {.width=16, .height=8, .size=32, .data=eeprom_buf};
            pw_flash_read(FLASH_IMG_FACE_HAPPY, face.data);
            pw_screen_draw_img(&face, (SCREEN_WIDTH-16)/2, (SCREEN_HEIGHT-8)/2);
            pw_screen_clear_area((SCREEN_WIDTH-8)/2, 48, 8, 8);

            if(sv->anim_frame&1) {
                pw_img_t img = {.width=8, .height=8, .size=16, .data=eeprom_buf};
                pw_flash_read(FLASH_IMG_IR_ACTIVE, img.data);
                pw_screen_draw_img(&img, (SCREEN_WIDTH-8)/2, 0);
            } else {
                pw_screen_clear_area((SCREEN_WIDTH-8)/2, 0, 8, 8);
            }
            break;
        }
    }
}

