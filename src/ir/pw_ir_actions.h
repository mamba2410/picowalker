#ifndef PW_IR_ACTIONS_H
#define PW_IR_ACTIONS_H

#include <stdint.h>
#include <stddef.h>

#include "pw_ir.h"
#include "trainer_info.h"

#define DECOMPRESSION_BUFFER_SIZE   256

typedef enum {
    COMM_SUBSTATE_NONE,
    COMM_SUBSTATE_FINDING_PEER,
    COMM_SUBSTATE_DETERMINE_ROLE,
    COMM_SUBSTATE_AWAITING_SLAVE_ACK,
    COMM_SUBSTATE_START_PEER_PLAY,
    COMM_SUBSTATE_PEER_PLAY_ACK,
    COMM_SUBSTATE_SEND_MASTER_SPRITES,
    COMM_SUBSTATE_SEND_MASTER_NAME_IMAGE,
    COMM_SUBSTATE_SEND_MASTER_TEAMDATA,
    COMM_SUBSTATE_READ_SLAVE_SPRITES,
    COMM_SUBSTATE_READ_SLAVE_NAME_IMAGE,
    COMM_SUBSTATE_READ_SLAVE_TEAMDATA,
    COMM_SUBSTATE_SEND_PEER_PLAY_DX,
    COMM_SUBSTATE_RECV_PEER_PLAY_DX,
    COMM_SUBSTATE_WRITE_PEER_PLAY_DATA,
    COMM_SUBSTATE_SEND_PEER_PLAY_END,
    COMM_SUBSTATE_RECV_PEER_PLAY_END,
    COMM_SUBSTATE_DISPLAY_PEER_PLAY_ANIMATION,
    COMM_SUBSTATE_CALCULATE_PEER_PLAY_GIFT,
    N_COMM_SUBSTATE,
} comm_substate_t;


ir_err_t pw_action_listen_and_advertise(uint8_t *rx, size_t *pn_read, uint8_t *padvertising_attempts);
ir_err_t pw_action_try_find_peer(uint8_t *packet, size_t packet_max,
        comm_substate_t *psubstate, uint8_t *padvertising_attempts);
ir_err_t pw_action_peer_play(comm_substate_t *psubstate, uint8_t *pcounter, uint8_t *packet, size_t max_len);
ir_err_t pw_action_slave_perform_request(uint8_t *packet, size_t len);

ir_err_t pw_action_send_large_raw_data_from_eeprom(uint16_t src, uint16_t dst, size_t final_write_size,
        size_t write_size, uint8_t *pcounter, uint8_t *packet, size_t max_len);
ir_err_t pw_action_read_large_raw_data_from_eeprom(uint16_t src, uint16_t dst, size_t final_read_size,
        size_t read_size, uint8_t *pcounter, uint8_t *packet, size_t max_len);
ir_err_t pw_action_send_large_raw_data_from_pointer(uint8_t *src, uint16_t dst, size_t final_write_size,
        size_t write_size, uint8_t *pcounter, uint8_t *packet, size_t max_len);

void pw_ir_end_walk();
ir_err_t pw_ir_eeprom_do_write(uint8_t *packet, size_t len);
void pw_log_event(event_log_item_t event_item);

#endif /* PW_IR_ACTIONS_H */
