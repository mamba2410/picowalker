#include <stdio.h>

#include <string.h> // memcpy()

#include "../eeprom_map.h"
#include "../eeprom.h"
#include "../trainer_info.h"
#include "../states.h"
#include "../rand.h"
#include "pw_ir.h"
#include "pw_ir_actions.h"
#include "compression.h"


static uint8_t decompression_buffer[DECOMPRESSION_BUFFER_SIZE];
ir_err_t pw_ir_eeprom_do_write(uint8_t *packet, size_t len);

/*
 *  Listen for a packet.
 *  If we don't hear anything, send advertising byte
 */
ir_err_t pw_action_listen_and_advertise(uint8_t *rx, size_t *pn_read, uint8_t *padvertising_attempts) {

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
ir_err_t pw_action_try_find_peer(state_vars_t *sv, uint8_t *packet, size_t packet_max) {

    ir_err_t err = IR_ERR_GENERAL;
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
                case IR_ERR_TIMEOUT: err = IR_OK; return IR_OK; // ignore timeout
                case IR_ERR_ADVERTISING_MAX: return IR_ERR_ADVERTISING_MAX;
                default: return IR_ERR_GENERAL;
            }

            //break;
        }
        case COMM_SUBSTATE_DETERMINE_ROLE: {

            // We should already have a response in the packet buffer
            switch(packet[0]) {
                case CMD_ADVERTISING: // we found peer, we request master

                    packet[0x00] = CMD_ASSERT_MASTER;
                    packet[0x01] = EXTRA_BYTE_FROM_WALKER;
                    err = pw_ir_send_packet(packet, 8, &n_read);

                    sv->current_substate = COMM_SUBSTATE_AWAITING_SLAVE_ACK;
                    break;
                case CMD_ASSERT_MASTER: // peer found us, peer requests master
                    packet[0x00] = CMD_SLAVE_ACK;
                    packet[0x01] = 2;

                    // record master key
                    uint8_t session_id_master[4];
                    for(int i = 0; i < 4; i++)
                        session_id_master[i] = packet[4+i];

                    err = pw_ir_send_packet(packet, 8, &n_read);

                    // combine keys
                    for(int i = 0; i < 4; i++)
                        session_id[i] ^= session_id_master[i];
                    pw_ir_delay_ms(1);

                    pw_ir_set_comm_state(COMM_STATE_SLAVE);
                    break;
                default: return IR_ERR_UNEXPECTED_PACKET;
            }
            break;
        }
        case COMM_SUBSTATE_AWAITING_SLAVE_ACK: {   // we have sent master request

            // wait for answer
            err = pw_ir_recv_packet(packet, 8, &n_read);
            if(err != IR_OK) return err;

            if(packet[0] != CMD_SLAVE_ACK) return IR_ERR_UNEXPECTED_PACKET;

            // combine keys
            for(int i = 0; i < 4; i++)
                session_id[i] ^= packet[4+i];

            // key exchange done, we are now master
            pw_ir_set_comm_state(COMM_STATE_MASTER);
            break;
        }
        default: return IR_ERR_UNKNOWN_SUBSTATE;
    }
    return err;
}


/*
 *  We are slave, given already recv'd packet, respond appropriately
 */
ir_err_t pw_action_slave_perform_request(uint8_t *packet, size_t len) {

    uint8_t cmd = packet[0];
    ir_err_t err = IR_ERR_GENERAL;
    size_t n_rw;

    switch(cmd) {
        case CMD_IDENTITY_REQ: {
            packet[0] = CMD_IDENTITY_RSP;
            packet[1] = EXTRA_BYTE_FROM_WALKER;

            int r = pw_eeprom_reliable_read(
                 PW_EEPROM_ADDR_IDENTITY_DATA_1,
                 PW_EEPROM_ADDR_IDENTITY_DATA_2,
                 packet+8,
                 PW_EEPROM_SIZE_IDENTITY_DATA_1
             );

            if(r < 0) {
                return IR_ERR_GENERAL;
            }

            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8+PW_EEPROM_SIZE_IDENTITY_DATA_1, &n_rw);

            break;
        }
        case CMD_IDENTITY_SEND_ALIAS1: {
            packet[0] = CMD_IDENTITY_ACK_ALIAS1;
            packet[1] = EXTRA_BYTE_TO_WALKER;

            //TODO: set the rtc, that's it

            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8, &n_rw);

            break;
        }
        case CMD_IDENTITY_SEND_ALIAS2: {
            packet[0] = CMD_IDENTITY_ACK_ALIAS2;
            packet[1] = EXTRA_BYTE_TO_WALKER;

            //TODO: set the rtc, that's it

            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8, &n_rw);

            break;
        }
        case CMD_IDENTITY_SEND_ALIAS3: {
            packet[0] = CMD_IDENTITY_ACK_ALIAS3;
            packet[1] = EXTRA_BYTE_TO_WALKER;

            //TODO: set the rtc, that's it

            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8, &n_rw);

            break;
        }

        case CMD_EEPROM_WRITE_CMP_00:
        case CMD_EEPROM_WRITE_RAW_00:
        case CMD_EEPROM_WRITE_CMP_80:
        case CMD_EEPROM_WRITE_RAW_80: {
            pw_ir_eeprom_do_write(packet, len);
            err = IR_OK;

            pw_ir_delay_ms(3);
            packet[0] = CMD_EEPROM_WRITE_ACK;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_ir_send_packet(packet, 8, &n_rw);
            break;
        }
        case CMD_EEPROM_READ_REQ: {
            uint16_t addr = packet[8]<<8 | packet[9];
            size_t len = packet[10];

            packet[0] = CMD_EEPROM_READ_RSP;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_eeprom_read(addr, packet+8, len);

            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8+len, &n_rw);
            break;
        }
        case CMD_PING: {
            packet[0] = CMD_PONG;
            packet[1] = EXTRA_BYTE_FROM_WALKER;

            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8, &n_rw);
            break;
        }
        case CMD_CONNECT_COMPLETE: {
            packet[0] = CMD_CONNECT_COMPLETE_ACK;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_ir_delay_ms(3);

            err = pw_ir_send_packet(packet, 8, &n_rw);
            break;
        }
        case 0x40: {
            packet[0] = 0x42;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_ir_delay_ms(3);
            err = pw_ir_send_packet(packet, 8, &n_rw);
            break;
        }
        case CMD_WALK_END_REQ: {
            packet[0] = CMD_WALK_END_ACK;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_ir_delay_ms(3);
            err = pw_ir_send_packet(packet, 8, &n_rw);

            pw_ir_end_walk();

            pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
            break;
        }
        case CMD_WALK_START: {
            packet[0] = CMD_WALK_START;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_ir_delay_ms(3);
            err = pw_ir_send_packet(packet, 8, &n_rw);
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
        default: {
            printf("[Error] Slave recv unhandled packet: %02x\n", cmd);
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
ir_err_t pw_action_peer_play(state_vars_t *sv, uint8_t *packet, size_t max_len) {
    ir_err_t err = IR_ERR_GENERAL;
    size_t n_read;

    switch(sv->current_substate) {
        case COMM_SUBSTATE_START_PEER_PLAY: {

            packet[0] = CMD_PEER_PLAY_START;
            packet[1] = EXTRA_BYTE_FROM_WALKER;
            pw_eeprom_reliable_read(PW_EEPROM_ADDR_IDENTITY_DATA_1, PW_EEPROM_ADDR_IDENTITY_DATA_2,
                    packet+8, PW_EEPROM_SIZE_IDENTITY_DATA_1);
            //pw_eeprom_read(PW_EEPROM_ADDR_IDENTITY_DATA_1,
            //        packet+8, PW_EEPROM_SIZE_IDENTITY_DATA_1);
            packet[0x18] = (uint8_t)(pw_rand()&0xff);  // Hack to change UID each time
                                                    // to prevent "already connected" error
                                                    // TODO: remove this in proper code
            err = pw_ir_send_packet(packet, 8+PW_EEPROM_SIZE_IDENTITY_DATA_1, &n_read);
            if(err != IR_OK) return err;

            sv->current_substate = COMM_SUBSTATE_PEER_PLAY_ACK;
            break;
        }
        case COMM_SUBSTATE_PEER_PLAY_ACK: {

            err = pw_ir_recv_packet(packet, 8+PW_EEPROM_SIZE_IDENTITY_DATA_1, &n_read);
            switch(packet[0]) {
                case CMD_PEER_PLAY_RSP: break;
                case CMD_PEER_PLAY_SEEN: return IR_ERR_PEER_ALREADY_SEEN;
                default: return IR_ERR_UNEXPECTED_PACKET;
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

            if(cur_write_size >= PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED){
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

            if(cur_write_size >= PW_EEPROM_SIZE_TEXT_POKEMON_NAME){
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

            if(cur_write_size >= PW_EEPROM_SIZE_TEAM_DATA_STRUCT){
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
            packet[0] = CMD_PEER_PLAY_DX;
            packet[1] = 1;

            // TODO: Actually make proper peer_play_data_t
            packet[0x08] = 0x0f;    // current steps = 9999
            packet[0x09] = 0x27;
            packet[0x0a] = 0;
            packet[0x0b] = 0;
            packet[0x0c] = 0x0f;    // current watts = 9999
            packet[0x0d] = 0x27;
            // 0x0e, 0x0f padding
            packet[0x10] = 1;   // identity_data_t.unk0
            packet[0x11] = 0;
            packet[0x12] = 0;
            packet[0x13] = 0;
            packet[0x14] = 7;   // identity_data_t.unk2
            packet[0x15] = 0;
            // species
            pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+0, packet+0x16, 2);
            // 22 bytes pokemon nickname
            pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+10, packet+0x18, 22);
            // 16 bytes trainer name
            pw_eeprom_read(PW_EEPROM_ADDR_IDENTITY_DATA_1+72, packet+0x2e, 16);
            // 1 byte pokemon gender
            pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+13, packet+0x3e, 1);
            // 1 byte pokeIsSpecial
            pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO+14, packet+0x3f, 1);

            // TODO: move sizze to #define
            err = pw_ir_send_packet(packet, 0x40, &n_read);;
            if(err != IR_OK) return err;

            sv->current_substate = COMM_SUBSTATE_RECV_PEER_PLAY_DX;
            break;
        }
        case COMM_SUBSTATE_RECV_PEER_PLAY_DX: {
            err = pw_ir_recv_packet(packet, 0x40, &n_read);
            if(err != IR_OK) return err;

            // TODO: Not 0x80-byte aligned
            pw_eeprom_write(PW_EEPROM_ADDR_CURRENT_PEER_DATA, packet, PW_EEPROM_SIZE_CURRENT_PEER_DATA);

            sv->current_substate = COMM_SUBSTATE_SEND_PEER_PLAY_END;
            break;
        }
        case COMM_SUBSTATE_SEND_PEER_PLAY_END: {
            packet[0] = CMD_PEER_PLAY_END;
            packet[1] = EXTRA_BYTE_TO_WALKER;
            err = pw_ir_send_packet(packet, 8, &n_read);

            sv->current_substate = COMM_SUBSTATE_RECV_PEER_PLAY_END;
            break;
        }
        case COMM_SUBSTATE_RECV_PEER_PLAY_END: {
            err = pw_ir_recv_packet(packet, 8, &n_read);
            if(err != IR_OK) return err;
            if(packet[0] != CMD_PEER_PLAY_END) return IR_ERR_UNEXPECTED_PACKET;
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
        size_t write_size, uint8_t *pcounter, uint8_t *packet, size_t max_len) {
    ir_err_t err = IR_ERR_GENERAL;

    size_t cur_write_size   = (size_t)(*pcounter) * write_size;
    uint16_t cur_write_addr = dst + cur_write_size;
    uint16_t cur_read_addr  = src + cur_write_size;
    size_t n_read = 0;

    // If we have written something, we expect an acknowledgment
    if(cur_write_size > 0) {
        err = pw_ir_recv_packet(packet, 8, &n_read);
        if(err != IR_OK) return err;
        if(packet[0] != CMD_EEPROM_WRITE_ACK) return IR_ERR_UNEXPECTED_PACKET;
    }

    if( (cur_write_addr&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;
    //if( (final_write_size&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;   // walker can handle this

    pw_ir_delay_ms(4);

    if( cur_write_size < final_write_size) {
        packet[0] = (uint8_t)(cur_write_addr&0xff) + 2; // Need +2 to make it raw write command
        packet[1] = (uint8_t)(cur_write_addr>>8);
        pw_eeprom_read(cur_read_addr, packet+8, write_size);

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
        size_t read_size, uint8_t *pcounter, uint8_t *packet, size_t max_len) {

    ir_err_t err;
    size_t cur_read_size   = (size_t)(*pcounter) * read_size;
    uint16_t cur_write_addr = dst + cur_read_size;
    uint16_t cur_read_addr  = src + cur_read_size;
    size_t n_read = 0;

    size_t remaining_read = final_read_size - cur_read_size;
    if(remaining_read <= 0) return IR_OK;

    read_size = (remaining_read<read_size)?remaining_read:read_size;

    packet[0] = CMD_EEPROM_READ_REQ;;
    packet[1] = EXTRA_BYTE_TO_WALKER;
    packet[8] = (uint8_t)(cur_read_addr>>8);
    packet[9] = (uint8_t)(cur_read_addr&0xff);
    packet[10] = read_size;

    err = pw_ir_send_packet(packet, 8+3, &n_read);
    if(err != IR_OK) return err;

    pw_ir_delay_ms(4);

    err = pw_ir_recv_packet(packet, read_size+8, &n_read);
    if(err != IR_OK) return err;
    if(packet[0] != CMD_EEPROM_READ_RSP) return IR_ERR_UNEXPECTED_PACKET;

    pw_eeprom_write(cur_write_addr, packet+8, read_size);

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
        size_t write_size, uint8_t *pcounter, uint8_t *packet, size_t max_len) {
    ir_err_t err = IR_ERR_GENERAL;

    size_t cur_write_size   = (size_t)(*pcounter) * write_size;
    uint16_t cur_write_addr = dst + cur_write_size;
    uint8_t *cur_read_addr  = src + cur_write_size;
    size_t n_read = 0;

    // If we have written something, we expect an acknowledgment
    if(cur_write_size > 0) {
        err = pw_ir_recv_packet(packet, 8, &n_read);
        if(err != IR_OK) return err;
        if(packet[0] != CMD_EEPROM_WRITE_ACK) return IR_ERR_UNEXPECTED_PACKET;
    }

    if( (cur_write_addr&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;
    //if( (final_write_size&0x07) > 0) return IR_ERR_UNALIGNED_WRITE;   // walker can handle this

    pw_ir_delay_ms(4);

    if( cur_write_size < final_write_size) {
        packet[0] = (uint8_t)(cur_write_addr&0xff) + 2; // Need +2 to make it raw write command
        packet[1] = (uint8_t)(cur_write_addr>>8);
        //pw_eeprom_read(cur_read_addr, packet+8, write_size);
        memcpy(packet+8, cur_read_addr, write_size);

        err = pw_ir_send_packet(packet, 8+write_size, &n_read);
        if(err != IR_OK) return err;
        (*pcounter)++;
    }

    return err;
}

ir_err_t pw_ir_eeprom_do_write(uint8_t *packet, size_t len) {
    ir_err_t err = IR_OK;
    uint8_t *data;
    uint8_t wlen = 128;

    uint8_t cmd = packet[0];
    uint16_t addr = (packet[1]<<8) | (cmd&0x80);

    if(!(cmd & 0x02)) {
        // decompress
        pw_decompress_data(packet+8, decompression_buffer, len-8);
        data = decompression_buffer;
    } else {
        data = packet+8;
    }

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
    info.flags &= ~0x2;

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

    // use decompression buffer as copy buffer
    uint8_t *buf = decompression_buffer;

    buf[0] = 0xa5;
    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_COPY_MARKER_1,
        PW_EEPROM_ADDR_COPY_MARKER_2,
        buf,
        1
    );

    for(size_t i = 0; i < 0x2900; i+=DECOMPRESSION_BUFFER_SIZE) {
        pw_eeprom_read(0xd700+i, buf, DECOMPRESSION_BUFFER_SIZE);
        pw_eeprom_write(0x8f00+i, buf, DECOMPRESSION_BUFFER_SIZE);
    }

    for(size_t i = 0; i < 0x280; i+=128) {
        pw_eeprom_read(0xd480+i, buf, 128);
        pw_eeprom_read(0xcc00+i, buf, 128);
    }

    buf[0] = 0x00;
    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_COPY_MARKER_1,
        PW_EEPROM_ADDR_COPY_MARKER_2,
        buf,
        1
    );


    pw_eeprom_set_area(PW_EEPROM_ADDR_EVENT_LOG, 0, PW_EEPROM_SIZE_EVENT_LOG);
    pw_eeprom_set_area(PW_EEPROM_ADDR_MET_PEER_DATA, 0, 0x1568);
    pw_eeprom_set_area(PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY, 0, 0x64);

    walker_info_t *info = (walker_info_t*)decompression_buffer;

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(info),
        PW_EEPROM_SIZE_IDENTITY_DATA_1
    );

    info->le_unk0 = 1;
    info->le_unk1 = 1;
    info->le_unk0 = 7;
    info->le_unk3 = 7;
    info->flags |= 0x2;

    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(info),
        PW_EEPROM_SIZE_IDENTITY_DATA_1
    );
    info = 0;

    // make walk start event


    route_info_t *route_info = (route_info_t*)decompression_buffer;
    event_log_item_t event_item = {0,};

    pw_eeprom_read(PW_EEPROM_ADDR_ROUTE_INFO, (uint8_t*)route_info, PW_EEPROM_SIZE_ROUTE_INFO);

    event_item.le_our_species = route_info->pokemon_summary.le_species;

    for(size_t i = 0; i < 11; i++)
        event_item.our_pokemon_name[i] = route_info->pokemon_nickname[i];

    event_item.route_image_index = route_info->route_image_index;
    event_item.event_type = 0x19;

    pw_log_event(event_item);


}

void pw_log_event(event_log_item_t event_item) {
    pw_eeprom_write(PW_EEPROM_ADDR_EVENT_LOG, (uint8_t*)(&event_item), sizeof(event_item));
}
