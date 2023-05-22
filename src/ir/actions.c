#include <stdio.h>
#include <stdbool.h>

#include <string.h> // memcpy()

#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../types.h"
#include "../states.h"
#include "../rand.h"
#include "ir.h"
#include "actions.h"
#include "compression.h"
#include "../globals.h"

#define ACTION_DELAY_MS 1

ir_err_t pw_ir_eeprom_do_write(pw_packet_t *packet, size_t len);
ir_err_t pw_ir_identity_ack(pw_packet_t *packet);

/*
 *  Listen for a packet.
 *  If we don't hear anything, send advertising byte
 */
ir_err_t pw_action_listen_and_advertise(pw_packet_t *rx, size_t *pn_read, uint8_t *padvertising_attempts) {

    ir_err_t err = IR_ERR_TIMEOUT;

    err = pw_ir_recv_packet(rx, 8, pn_read);

    // if we didn't read anything, send an advertising packet
    if(*pn_read == 0) {
        (void)pw_ir_send_advertising_packet();

        (*padvertising_attempts)++;
        if(*padvertising_attempts > MAX_ADVERTISING_PACKETS) {
            return IR_ERR_ADVERTISING_MAX;
        }
    }

    return err;
}


/*
 *  We should be in COMM_STATE_AWAITING
 *  Do one action per call, called in the main event loop
 */
ir_err_t pw_action_try_find_peer(state_vars_t *sv, pw_packet_t *packet, size_t packet_max) {

    ir_err_t err = IR_ERR_UNHANDLED_ERROR;
    size_t n_read = 0;

    switch(sv->current_substate) {
    case COMM_SUBSTATE_FINDING_PEER: {

        err = pw_action_listen_and_advertise(packet, &n_read, &(sv->reg_c));

        switch(err) {
        case IR_ERR_SIZE_MISMATCH:  // also ok since we might recv 0xfc
        case IR_OK:
            // we got a valid packet back, now check if master or slave on next iteration
            sv->current_substate = COMM_SUBSTATE_DETERMINE_ROLE;
            break;
        case IR_ERR_TIMEOUT:
            err = IR_OK;
            return IR_OK; // ignore timeout
        case IR_ERR_ADVERTISING_MAX:
            return IR_ERR_ADVERTISING_MAX;
        default:
            return err; // TODO: change this
        }

        //break;
    }
    case COMM_SUBSTATE_DETERMINE_ROLE: {

        // We should already have a response in the packet buffer
        switch(packet->cmd) {
        case CMD_ADVERTISING: // we found peer, we request master

            packet->cmd = CMD_ASSERT_MASTER;
            packet->extra = EXTRA_BYTE_FROM_WALKER;
            err = pw_ir_send_packet(packet, 8, &n_read);

            sv->current_substate = COMM_SUBSTATE_AWAITING_SLAVE_ACK;
            break;
        case CMD_ASSERT_MASTER: // peer found us, peer requests master
            packet->cmd = CMD_SLAVE_ACK;
            packet->extra = 2;

            // record master key
            uint8_t session_id_master[4];
            for(int i = 0; i < 4; i++)
                session_id_master[i] = packet->session_id_bytes[i];

            err = pw_ir_send_packet(packet, 8, &n_read);

            // combine keys
            for(int i = 0; i < 4; i++)
                session_id[i] ^= session_id_master[i];
            pw_ir_delay_ms(ACTION_DELAY_MS);

            pw_ir_set_comm_state(COMM_STATE_SLAVE);
            break;
        default:
            return IR_ERR_UNEXPECTED_PACKET;
        }
        break;
    }
    case COMM_SUBSTATE_AWAITING_SLAVE_ACK: {   // we have sent master request

        // wait for answer
        err = pw_ir_recv_packet(packet, 8, &n_read);
        if(err != IR_OK) return err;

        if(packet->cmd != CMD_SLAVE_ACK) return IR_ERR_UNEXPECTED_PACKET;

        // combine keys
        for(int i = 0; i < 4; i++)
            session_id[i] ^= packet->session_id_bytes[i];

        // key exchange done, we are now master
        pw_ir_set_comm_state(COMM_STATE_MASTER);
        break;
    }
    default:
        return IR_ERR_UNKNOWN_SUBSTATE;
    }
    return err;
}


/*
 *  We are slave, given already recv'd packet, respond appropriately
 */
ir_err_t pw_action_slave_perform_request(pw_packet_t *packet, size_t len) {

    ir_err_t err = IR_ERR_UNHANDLED_ERROR;
    size_t n_rw;

    switch(packet->cmd) {
    case CMD_IDENTITY_REQ: {
        packet->cmd = CMD_IDENTITY_RSP;
        packet->extra = EXTRA_BYTE_FROM_WALKER;

        int r = pw_eeprom_reliable_read(
                    PW_EEPROM_ADDR_IDENTITY_DATA_1,
                    PW_EEPROM_ADDR_IDENTITY_DATA_2,
                    packet->payload,
                    PW_EEPROM_SIZE_IDENTITY_DATA_1
                );

        if(r < 0) {
            return IR_ERR_BAD_DATA;
        }

        pw_ir_delay_ms(ACTION_DELAY_MS);

        err = pw_ir_send_packet(packet, 8+PW_EEPROM_SIZE_IDENTITY_DATA_1, &n_rw);

        break;
    }
    case CMD_IDENTITY_SEND:
    case CMD_IDENTITY_SEND_ALIAS1:
    case CMD_IDENTITY_SEND_ALIAS2:
    case CMD_IDENTITY_SEND_ALIAS3: {
        err = pw_ir_identity_ack(packet);
        break;
    }
    case CMD_EEPROM_WRITE_CMP_00:
    case CMD_EEPROM_WRITE_RAW_00:
    case CMD_EEPROM_WRITE_CMP_80:
    case CMD_EEPROM_WRITE_RAW_80: {
        err = pw_ir_eeprom_do_write(packet, len);

        pw_ir_delay_ms(ACTION_DELAY_MS);
        packet->cmd = CMD_EEPROM_WRITE_ACK;
        packet->extra = EXTRA_BYTE_FROM_WALKER;
        pw_ir_send_packet(packet, 8, &n_rw);
        break;
    }
    case CMD_EEPROM_READ_REQ: {
        uint16_t addr = packet->payload[0]<<8 | packet->payload[1];
        size_t len = packet->payload[2];

        packet->cmd = CMD_EEPROM_READ_RSP;
        packet->extra = EXTRA_BYTE_FROM_WALKER;
        pw_eeprom_read(addr, packet->payload, len);

        pw_ir_delay_ms(ACTION_DELAY_MS);

        err = pw_ir_send_packet(packet, 8+len, &n_rw);
        break;
    }
    case CMD_PING: {
        packet->cmd = CMD_PONG;
        packet->extra = EXTRA_BYTE_FROM_WALKER;

        pw_ir_delay_ms(ACTION_DELAY_MS);

        err = pw_ir_send_packet(packet, 8, &n_rw);
        break;
    }
    case CMD_CONNECT_COMPLETE: {
        packet->cmd = CMD_CONNECT_COMPLETE_ACK;
        packet->cmd = EXTRA_BYTE_FROM_WALKER;
        pw_ir_delay_ms(ACTION_DELAY_MS);

        err = pw_ir_send_packet(packet, 8, &n_rw);
        break;
    }
    case CMD_WALK_END_REQ: {
        packet->cmd = CMD_WALK_END_ACK;
        packet->extra = EXTRA_BYTE_FROM_WALKER;
        pw_ir_delay_ms(ACTION_DELAY_MS);
        err = pw_ir_send_packet(packet, 8, &n_rw);

        pw_ir_end_walk();

        pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
        break;
    }
    case CMD_WALK_START_INIT:
    case CMD_WALK_START: {
        // keep cmd
        packet->extra = EXTRA_BYTE_FROM_WALKER;
        pw_ir_delay_ms(ACTION_DELAY_MS);
        err = pw_ir_send_packet(packet, 8, &n_rw);
        pw_ir_start_walk();
        break;
    }
    case CMD_DISCONNECT: {
        err = IR_OK;
        pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
        break;
    }
    case CMD_NOCOMPLETE_ALIAS1: {
        err = IR_OK;
        pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
        break;
    }
    case CMD_WALKER_RESET_1: {
        packet->extra = EXTRA_BYTE_FROM_WALKER;
        pw_ir_delay_ms(ACTION_DELAY_MS);
        pw_eeprom_reliable_read(
            PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_1,
            PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_2,
            packet->payload,
            sizeof(unique_identity_data_t)
        );
        err = pw_ir_send_packet(packet, 8+sizeof(unique_identity_data_t), &n_rw);
        pw_eeprom_reset(true, false);
        break;
    }
    default: {
        printf("[Error] Slave recv unhandled packet: %02x\n", packet->cmd);
        err = IR_ERR_UNEXPECTED_PACKET;
        break;
    }

    }

    return err;
}

/*
 *  Sequence:
 *  send CMD_PEER_PLAY_START
 *  recv CMD_PEER_PLAY_RSP
 *  send master EEPROM:0x91BE to slave EEPROM:0xF400
 *  send master EEPROM:0xCC00 to slave EEPROM:0xDC00
 *  read slave EEPROM:0x91BE to master EEPROM:0xF400
 *  read slave EEPROM:0xCC00 to master EEPROM:0xDC00
 *  send CMD_PEER_PLAY_DX
 *  recv CMD_PEER_PLAY_DX ?
 *  write data to master EEPROM:0xF6C0
 *  send CMD_PEER_PLAY_END
 *  recv CMD_PEER_PLAY_END
 *  display animation
 *  calculate gift
 */
ir_err_t pw_action_peer_play(state_vars_t *sv, pw_packet_t *packet, size_t max_len) {
    ir_err_t err = IR_ERR_UNHANDLED_ERROR;
    size_t n_read;

    switch(sv->current_substate) {
    case COMM_SUBSTATE_START_PEER_PLAY: {

        packet->cmd = CMD_PEER_PLAY_START;
        packet->extra = EXTRA_BYTE_FROM_WALKER;
        pw_eeprom_reliable_read(PW_EEPROM_ADDR_IDENTITY_DATA_1, PW_EEPROM_ADDR_IDENTITY_DATA_2,
                                packet->payload, PW_EEPROM_SIZE_IDENTITY_DATA_1);
        //pw_eeprom_read(PW_EEPROM_ADDR_IDENTITY_DATA_1,
        //        packet+8, PW_EEPROM_SIZE_IDENTITY_DATA_1);
        packet->bytes[0x18] = (uint8_t)(pw_rand()&0xff);  // Hack to change UID each time
        // to prevent "already connected" error
        // TODO: remove this in proper code
        err = pw_ir_send_packet(packet, 8+PW_EEPROM_SIZE_IDENTITY_DATA_1, &n_read);
        if(err != IR_OK) return err;

        sv->current_substate = COMM_SUBSTATE_PEER_PLAY_ACK;
        break;
    }
    case COMM_SUBSTATE_PEER_PLAY_ACK: {

        err = pw_ir_recv_packet(packet, 8+PW_EEPROM_SIZE_IDENTITY_DATA_1, &n_read);
        switch(packet->cmd) {
        case CMD_PEER_PLAY_RSP:
            break;
        case CMD_PEER_PLAY_SEEN:
            return IR_ERR_PEER_ALREADY_SEEN;
        default:
            return IR_ERR_UNEXPECTED_PACKET;
        }
        if(err != IR_OK) return err;

        sv->current_substate = COMM_SUBSTATE_SEND_MASTER_SPRITES;
        sv->reg_c = 0; // reset loop counter
        break;
    }
    case COMM_SUBSTATE_SEND_MASTER_SPRITES: {

        size_t write_size = 128;    // should always be 128-bytes
        size_t cur_write_size   = (size_t)(sv->reg_c) * write_size;

        err = pw_action_send_large_raw_data_from_eeprom(
                  PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED,              // src
                  PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL, // dst
                  PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED,              // size
                  write_size, &(sv->reg_c), packet, max_len
              );
        if(err != IR_OK) return err;

        if(cur_write_size >= PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED) {
            sv->reg_c = 0;  // reset loop counter
            sv->current_substate = COMM_SUBSTATE_SEND_MASTER_NAME_IMAGE;
        }
        break;
    }
    case COMM_SUBSTATE_SEND_MASTER_NAME_IMAGE: {

        size_t write_size = 128;
        size_t cur_write_size   = (size_t)(sv->reg_c) * write_size;

        err = pw_action_send_large_raw_data_from_eeprom(
                  PW_EEPROM_ADDR_TEXT_POKEMON_NAME,               // src
                  PW_EEPROM_ADDR_TEXT_CURRENT_PEER_POKEMON_NAME,  // dst
                  PW_EEPROM_SIZE_TEXT_POKEMON_NAME,               // size
                  write_size, &(sv->reg_c), packet, max_len
              );

        if(cur_write_size >= PW_EEPROM_SIZE_TEXT_POKEMON_NAME) {
            sv->reg_c = 0; // reset loop counter
            sv->current_substate = COMM_SUBSTATE_SEND_MASTER_TEAMDATA;
        }
        break;
    }
    case COMM_SUBSTATE_SEND_MASTER_TEAMDATA: {

        size_t write_size = 128;
        size_t cur_write_size   = (size_t)(sv->reg_c) * write_size;

        err = pw_action_send_large_raw_data_from_eeprom(
                  PW_EEPROM_ADDR_TEAM_DATA_STRUCT,            // src
                  PW_EEPROM_ADDR_CURRENT_PEER_TEAM_DATA,      // dst
                  PW_EEPROM_SIZE_TEAM_DATA_STRUCT,            // size
                  write_size, &(sv->reg_c), packet, max_len
              );

        if(cur_write_size >= PW_EEPROM_SIZE_TEAM_DATA_STRUCT) {
            sv->reg_c = 0; // reset loop counter
            sv->current_substate = COMM_SUBSTATE_READ_SLAVE_SPRITES;
        }
        break;
    }
    case COMM_SUBSTATE_READ_SLAVE_SPRITES: {

        size_t read_size = 128; // NOTE: this can be anything so long as it fits in your buffer
        // TODO: move this buffer dependancy inti pw_ir_read()
        err = pw_action_read_large_raw_data_from_eeprom(
                  PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED,              // src
                  PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL, // dst
                  PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED,              // size
                  read_size, &(sv->reg_c), packet, max_len
              );

        size_t cur_read_size   = (size_t)(sv->reg_c) * read_size;

        if(cur_read_size >= PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED) {
            sv->reg_c = 0;  // reset loop counter
            sv->current_substate = COMM_SUBSTATE_READ_SLAVE_NAME_IMAGE;
        }
        break;
    }
    case COMM_SUBSTATE_READ_SLAVE_NAME_IMAGE: {

        size_t read_size = 128;  // TODO: See above
        err = pw_action_read_large_raw_data_from_eeprom(
                  PW_EEPROM_ADDR_TEXT_POKEMON_NAME,               // src
                  PW_EEPROM_ADDR_TEXT_CURRENT_PEER_POKEMON_NAME,  // dst
                  PW_EEPROM_SIZE_TEXT_POKEMON_NAME,               // size
                  read_size, &(sv->reg_c), packet, max_len
              );

        size_t cur_read_size   = (size_t)(sv->reg_c) * read_size;

        if(cur_read_size >= PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED) {
            sv->reg_c = 0;  // reset loop counter
            sv->current_substate = COMM_SUBSTATE_READ_SLAVE_TEAMDATA;
        }
        break;
    }
    case COMM_SUBSTATE_READ_SLAVE_TEAMDATA: {

        size_t read_size = 128;  // TODO: See above
        err = pw_action_read_large_raw_data_from_eeprom(
                  PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED,              // src
                  PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL, // dst
                  PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED,              // size
                  read_size, &(sv->reg_c), packet, max_len
              );

        size_t cur_read_size   = (size_t)(sv->reg_c) * read_size;

        if(cur_read_size >= PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED) {
            sv->reg_c = 0;  // reset loop counter
            sv->current_substate = COMM_SUBSTATE_SEND_PEER_PLAY_DX;
        }
        break;
    }
    case COMM_SUBSTATE_SEND_PEER_PLAY_DX: {
        packet->cmd = CMD_PEER_PLAY_DX;
        packet->extra = 1;

        // TODO: Actually make proper peer_play_data_t
        // eg peer_play_data_t *ppd = packet->payload
        packet->payload[0x00] = 0x0f;    // current steps = 9999
        packet->payload[0x01] = 0x27;
        packet->payload[0x02] = 0;
        packet->payload[0x03] = 0;
        packet->payload[0x04] = 0x0f;    // current watts = 9999
        packet->payload[0x05] = 0x27;
        // 0x0e, 0x0f padding
        packet->payload[0x08] = 1;   // identity_data_t.unk0
        packet->payload[0x09] = 0;
        packet->payload[0x0a] = 0;
        packet->payload[0x0b] = 0;
        packet->payload[0x0c] = 7;   // identity_data_t.unk2
        packet->payload[0x0d] = 0;
        // species
        pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+0, packet->bytes+0x16, 2);
        // 22 bytes pokemon nickname
        pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+10, packet->bytes+0x18, 22);
        // 16 bytes trainer name
        pw_eeprom_read(PW_EEPROM_ADDR_IDENTITY_DATA_1+72, packet->bytes+0x2e, 16);
        // 1 byte pokemon gender
        pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+13, packet->bytes+0x3e, 1);
        // 1 byte pokeIsSpecial
        pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+14, packet->bytes+0x3f, 1);

        // TODO: move sizze to #define
        err = pw_ir_send_packet(packet, 0x40, &n_read);;
        if(err != IR_OK) return err;

        sv->current_substate = COMM_SUBSTATE_RECV_PEER_PLAY_DX;
        break;
    }
    case COMM_SUBSTATE_RECV_PEER_PLAY_DX: {
        err = pw_ir_recv_packet(packet, 0x40, &n_read);
        if(err != IR_OK) return err;

        pw_eeprom_write(PW_EEPROM_ADDR_CURRENT_PEER_DATA, packet->payload, PW_EEPROM_SIZE_CURRENT_PEER_DATA);

        sv->current_substate = COMM_SUBSTATE_SEND_PEER_PLAY_END;
        break;
    }
    case COMM_SUBSTATE_SEND_PEER_PLAY_END: {
        packet->cmd = CMD_PEER_PLAY_END;
        packet->extra = EXTRA_BYTE_TO_WALKER;
        err = pw_ir_send_packet(packet, 8, &n_read);

        sv->current_substate = COMM_SUBSTATE_RECV_PEER_PLAY_END;
        break;
    }
    case COMM_SUBSTATE_RECV_PEER_PLAY_END: {
        err = pw_ir_recv_packet(packet, 8, &n_read);
        if(err != IR_OK) return err;
        if(packet->cmd != CMD_PEER_PLAY_END) return IR_ERR_UNEXPECTED_PACKET;
        sv->current_substate = COMM_SUBSTATE_DISPLAY_PEER_PLAY_ANIMATION;
        break;
    }
    case COMM_SUBSTATE_DISPLAY_PEER_PLAY_ANIMATION: {
        err = IR_ERR_NOT_IMPLEMENTED;
        break;
    }
    case COMM_SUBSTATE_CALCULATE_PEER_PLAY_GIFT: {
        err = IR_ERR_NOT_IMPLEMENTED;
        break;
    }
    default:
        err = IR_ERR_UNKNOWN_SUBSTATE;
        break;
    }

    return err;
}


/*
 *  Send an eeprom section from `src` on host to `dst` on peer.
 *  Throws error if `dst` or `final_write_size` isn't 128-byte aligned
 *
 *  Designed to be run in a loop, hence only one read and one write.
 */
ir_err_t pw_action_send_large_raw_data_from_eeprom(uint16_t src, uint16_t dst, size_t final_write_size,
        size_t write_size, uint8_t *pcounter, pw_packet_t *packet, size_t max_len) {
    ir_err_t err = IR_ERR_UNHANDLED_ERROR;

    size_t cur_write_size   = (size_t)(*pcounter) * write_size;
    uint16_t cur_write_addr = dst + cur_write_size;
    uint16_t cur_read_addr  = src + cur_write_size;
    size_t n_read = 0;

    // If we have written something, we expect an acknowledgment
    if(cur_write_size > 0) {
        err = pw_ir_recv_packet(packet, 8, &n_read);
        if(err != IR_OK) return err;
        if(packet->cmd != CMD_EEPROM_WRITE_ACK) return IR_ERR_UNEXPECTED_PACKET;
    }

    if( (cur_write_addr&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;
    //if( (final_write_size&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;   // walker can handle this

    pw_ir_delay_ms(ACTION_DELAY_MS);

    if( cur_write_size < final_write_size) {
        packet->cmd = (uint8_t)(cur_write_addr&0xff) + 2; // Need +2 to make it raw write command
        packet->extra = (uint8_t)(cur_write_addr>>8);
        pw_eeprom_read(cur_read_addr, packet->payload, write_size);

        err = pw_ir_send_packet(packet, 8+write_size, &n_read);
        if(err != IR_OK) return err;
        (*pcounter)++;
    }

    return err;
}


/*
 *  Send an eeprom section from `src` on peer to `dst` on host.
 *
 *  Designed to be run in a loop, hence only one read and one write.
 */
ir_err_t pw_action_read_large_raw_data_from_eeprom(uint16_t src, uint16_t dst, size_t final_read_size,
        size_t read_size, uint8_t *pcounter, pw_packet_t *packet, size_t max_len) {

    ir_err_t err;
    size_t cur_read_size   = (size_t)(*pcounter) * read_size;
    uint16_t cur_write_addr = dst + cur_read_size;
    uint16_t cur_read_addr  = src + cur_read_size;
    size_t n_read = 0;

    size_t remaining_read = final_read_size - cur_read_size;
    if(remaining_read <= 0) return IR_OK;

    read_size = (remaining_read<read_size)?remaining_read:read_size;

    packet->cmd = CMD_EEPROM_READ_REQ;;
    packet->extra = EXTRA_BYTE_TO_WALKER;
    packet->payload[0] = (uint8_t)(cur_read_addr>>8);
    packet->payload[1] = (uint8_t)(cur_read_addr&0xff);
    packet->payload[2] = read_size;

    err = pw_ir_send_packet(packet, 8+3, &n_read);
    if(err != IR_OK) return err;

    pw_ir_delay_ms(ACTION_DELAY_MS);

    err = pw_ir_recv_packet(packet, read_size+8, &n_read);
    if(err != IR_OK) return err;
    if(packet->cmd != CMD_EEPROM_READ_RSP) return IR_ERR_UNEXPECTED_PACKET;

    pw_eeprom_write(cur_write_addr, packet->payload, read_size);

    (*pcounter)++;

    // TODO: create a new error for awaiting read/write?
    return err;
}

/*
 * Send a contiguous section of data from `src` ptr to `dst` eeprom address
 *
 *  Designed to be run in a loop, hence only one read and one write.
 */
ir_err_t pw_action_send_large_raw_data_from_pointer(uint8_t *src, uint16_t dst, size_t final_write_size,
        size_t write_size, uint8_t *pcounter, pw_packet_t *packet, size_t max_len) {
    ir_err_t err = IR_ERR_UNHANDLED_ERROR;

    size_t cur_write_size   = (size_t)(*pcounter) * write_size;
    uint16_t cur_write_addr = dst + cur_write_size;
    uint8_t *cur_read_addr  = src + cur_write_size;
    size_t n_read = 0;

    // If we have written something, we expect an acknowledgment
    if(cur_write_size > 0) {
        err = pw_ir_recv_packet(packet, 8, &n_read);
        if(err != IR_OK) return err;
        if(packet->cmd != CMD_EEPROM_WRITE_ACK) return IR_ERR_UNEXPECTED_PACKET;
    }

    if( (cur_write_addr&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;
    //if( (final_write_size&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;   // walker can handle this

    pw_ir_delay_ms(ACTION_DELAY_MS);

    if( cur_write_size < final_write_size) {
        packet->cmd = (uint8_t)(cur_write_addr&0xff) + 2; // Need +2 to make it raw write command
        packet->extra = (uint8_t)(cur_write_addr>>8);
        //pw_eeprom_read(cur_read_addr, packet+8, write_size);
        memcpy(packet->payload, cur_read_addr, write_size);

        err = pw_ir_send_packet(packet, 8+write_size, &n_read);
        if(err != IR_OK) return err;
        (*pcounter)++;
    }

    return err;
}

ir_err_t pw_ir_eeprom_do_write(pw_packet_t *packet, size_t len) {
    ir_err_t err = IR_OK;
    uint8_t *data;
    uint8_t wlen = 128;

    uint8_t cmd = packet->cmd;
    uint16_t addr = (packet->extra<<8) | (cmd&0x80);
    // compressed if 0x00 or 0x02 and length < 136
    bool cmp = ( (cmd&0x02) == 0 ) && (len<0x88);

    //printf("P %02x %02x, len:0x%02x ; addr:%04x cmp:%d", cmd, packet[1], len, addr, cmp);

    //printf("addr: %04x", addr);
    if(cmp) {
        // decompress
        int e = pw_decompress_data(packet->payload, decompression_buf, len-8);
        if(e != 0) return IR_ERR_BAD_DATA;
        data = decompression_buf;
    } else {
        data = packet->payload;
    }

    if(addr == 0xd700) {
        printf("decomp species: %02x%02x\n", data[1], data[0]);
    }

    //printf("\n");
    pw_eeprom_write(addr, data, wlen);

    return err;
}


void pw_ir_end_walk() {

    walker_info_t info;

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(&info),
        PW_EEPROM_SIZE_IDENTITY_DATA_1
    );

    info.le_unk1 = 0;
    info.le_unk3 = 0;
    info.flags &= ~WALKER_INFO_FLAG_HAS_POKEMON;

    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(&info),
        PW_EEPROM_SIZE_IDENTITY_DATA_1
    );


    pw_eeprom_set_area(PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY, 0, 0x64);
    pw_eeprom_set_area(PW_EEPROM_ADDR_EVENT_LOG, 0, PW_EEPROM_SIZE_EVENT_LOG);
    pw_eeprom_set_area(PW_EEPROM_ADDR_RECEIVED_BITFIELD, 0, 0x6c8);
    pw_eeprom_set_area(PW_EEPROM_ADDR_MET_PEER_DATA, 0, 0x1568);
    pw_eeprom_set_area(PW_EEPROM_ADDR_ROUTE_INFO, 0, 0x10);

}


void pw_ir_start_walk() {

    uint8_t *buf = eeprom_buf;
    size_t buf_size = EEPROM_BUF_SIZE;
    int n = 0;

    buf[0] = 0xa5;
    n = pw_eeprom_reliable_write(
            PW_EEPROM_ADDR_COPY_MARKER_1,
            PW_EEPROM_ADDR_COPY_MARKER_2,
            buf,
            1
        );

    // buf_size must wholly divide into copy size
    const size_t sz = 128;
    for(size_t i = 0; i < 0x2900; i+=sz) {
        // Sometimes reads zero here for x amount of bytes
        // but later on line 746, it reads ok
        int a = pw_eeprom_read(PW_EEPROM_ADDR_SCENARIO_STAGING_AREA+i, buf, sz);
        uint16_t species = buf[0] | (uint16_t)(buf[1])<<8;
        if(i == 0 && species == 0) {
            printf("n read: %d\n", a);
        }
        pw_eeprom_write(PW_EEPROM_ADDR_ROUTE_INFO+i, buf, sz);
    }

    for(size_t i = 0; i < 0x280; i+=sz) {
        pw_eeprom_read(PW_EEPROM_ADDR_TEAM_DATA_STAGING+i, buf, sz);
        pw_eeprom_read(PW_EEPROM_ADDR_TEAM_DATA_STRUCT+i,  buf, sz);
    }

    buf[0] = 0x00;
    n = pw_eeprom_reliable_write(
            PW_EEPROM_ADDR_COPY_MARKER_1,
            PW_EEPROM_ADDR_COPY_MARKER_2,
            buf,
            1
        );

    health_data_cache.walk_minute_counter = 0;
    health_data_cache.event_log_index = 0;
    health_data_cache.current_watts = 0;

    n = pw_eeprom_reliable_write(
            PW_EEPROM_ADDR_HEALTH_DATA_1,
            PW_EEPROM_ADDR_HEALTH_DATA_2,
            (uint8_t*)&health_data_cache,
            sizeof(health_data_cache)
        );

    // this always reads ok, so the write must have been fine
    route_info_t *route_info = (route_info_t*)buf;
    pw_eeprom_read(PW_EEPROM_ADDR_SCENARIO_STAGING_AREA, (uint8_t*)route_info, PW_EEPROM_SIZE_ROUTE_INFO);
    printf("d700 species: %04x\n", route_info->pokemon_summary.le_species);


    pw_eeprom_set_area(PW_EEPROM_ADDR_EVENT_LOG, 0, PW_EEPROM_SIZE_EVENT_LOG);
    pw_eeprom_set_area(PW_EEPROM_ADDR_MET_PEER_DATA, 0, 0x1568);
    pw_eeprom_set_area(PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY, 0, 0x64);

    //walker_info_t *info = (walker_info_t*)buf;
    walker_info_t *info = &walker_info_cache;

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(info),
        PW_EEPROM_SIZE_IDENTITY_DATA_1
    );


    info->le_unk0 = peer_info_cache.le_unk0;
    info->le_unk1 = info->le_unk0;
    info->le_unk2 = peer_info_cache.le_unk2;
    info->le_unk3 = info->le_unk2;

    info->flags &= ~WALKER_INFO_FLAG_POKEMON_JOINED;
    info->flags |= WALKER_INFO_FLAG_INIT | WALKER_INFO_FLAG_HAS_POKEMON;

    info->le_tid = peer_info_cache.le_tid;
    info->le_sid = peer_info_cache.le_sid;

    for(size_t i = 0; i < 8; i++) {
        info->le_trainer_name[i] = peer_info_cache.le_trainer_name[i];
    }

    info->identity_data = peer_info_cache.identity_data;

    info->protocol_ver = peer_info_cache.protocol_ver;
    info->protocol_subver = peer_info_cache.protocol_subver;
    info->unk5 = peer_info_cache.unk5;
    info->unk8 = 0x02;

    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(info),
        PW_EEPROM_SIZE_IDENTITY_DATA_1
    );
    info = 0;


    // make walk start event

    event_log_item_t *event_item = malloc(sizeof(*event_item));

    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)route_info, PW_EEPROM_SIZE_ROUTE_INFO);
    printf("8f00 species: %04x\n", route_info->pokemon_summary.le_species);
    //pw_eeprom_read(0xd700, (uint8_t*)route_info, PW_EEPROM_SIZE_ROUTE_INFO);
    //printf("d700 species: %04x\n", route_info->pokemon_summary.le_species);


    event_item->le_our_species = route_info->pokemon_summary.le_species;
    event_item->our_pokemon_flags = route_info->pokemon_summary.pokemon_flags_1;
    for(uint8_t i = 0; i < 11; i++)
        event_item->our_pokemon_name[i] = route_info->pokemon_nickname[i];
    printf("event species: %04x\n", event_item->le_our_species);

    for(size_t i = 0; i < 11; i++)
        event_item->our_pokemon_name[i] = route_info->pokemon_nickname[i];

    event_item->route_image_index = route_info->route_image_index;
    event_item->event_type = 0x19;

    pw_log_event(event_item);

    free(event_item);
}

void pw_log_event(event_log_item_t *event_item) {
    pw_eeprom_write(PW_EEPROM_ADDR_EVENT_LOG, (uint8_t*)event_item, sizeof(*event_item));
}


ir_err_t pw_ir_identity_ack(pw_packet_t *packet) {
    size_t n_rw;
    switch(packet->cmd) {
    case CMD_IDENTITY_SEND:
        packet->cmd = CMD_IDENTITY_ACK;
        break;
    case CMD_IDENTITY_SEND_ALIAS1:
        packet->cmd = CMD_IDENTITY_ACK_ALIAS1;
        break;
    case CMD_IDENTITY_SEND_ALIAS2:
        packet->cmd = CMD_IDENTITY_ACK_ALIAS2;
        break;
    case CMD_IDENTITY_SEND_ALIAS3:
        packet->cmd = CMD_IDENTITY_ACK_ALIAS3;
        break;
    default:
        return IR_ERR_UNEXPECTED_PACKET;
    }

    for(size_t i = 0; i < sizeof(walker_info_t); i++) {
        ((uint8_t*)(&peer_info_cache))[i] = packet->payload[i];
    }

    packet->extra = EXTRA_BYTE_TO_WALKER;

    //TODO: set the rtc, that's it

    pw_ir_delay_ms(ACTION_DELAY_MS);

    ir_err_t err = pw_ir_send_packet(packet, 8, &n_rw);
    return err;
}

