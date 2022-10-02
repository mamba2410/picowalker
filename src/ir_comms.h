#ifndef IR_COMMS_H
#define IR_COMMS_H

#include "ir_commands.h"

#include <stdint.h>
#include <stddef.h>

#define RX_BUF_LEN	(128+PACKET_HEADER_LEN)
#define TX_BUF_LEN	(128+PACKET_HEADER_LEN)
#define MAX_ADVERTISING_ATTEMPTS 20

#define CMD_EEPROM_READ_REQ     0x0c
#define CMD_EEPROM_READ_RSP     0x0e
#define CMD_EEPROM_WRITE_CMP_00 0x00    // eeprom write compressed
#define CMD_EEPROM_WRITE_RAW_00 0x02    // eeprom write directly
#define CMD_EEPROM_WRITE_CMP_80 0x80    // eeprom write compressed
#define CMD_EEPROM_WRITE_RAW_80 0x82    // eeprom write directly
#define CMD_EEPROM_WRITE_RND    0x0a    // eeprom write random length
#define CMD_EEPROM_WRITE_ACK    0x04

#define CMD_PEER_PLAY_START     0x10
#define CMD_PEER_PLAY_RSP       0x12
#define CMD_PEER_PLAY_DX        0x14    // peer_play_data_t exchange
#define CMD_PEER_PLAY_END       0x16
#define CMD_PEER_PLAY_SEEN      0x1c    // sent if peer was seen too recently

#define CMD_IDENTITY_REQ        0x20    // game requesting identity data
#define CMD_IDENTITY_RSP        0x22
#define CMD_IDENTITY_SEND       0x32    // master's identity to slave
#define CMD_IDENTITY_ACK        0x34
#define CMD_IDENTITY_SEND_ALIAS1 0x42    // alias for 0x32
#define CMD_IDENTITY_ACK_ALIAS1  0x44    // resp for 0x42
#define CMD_IDENTITY_SEND_ALIAS2 0x52    // alias for 0x32
#define CMD_IDENTITY_ACK_ALIAS2  0x54    // resp for 0x52
#define CMD_IDENTITY_SEND_ALIAS3 0x60    // alias for 0x32
#define CMD_IDENTITY_ACK_ALIAS3  0x62    // resp for 0x60

#define CMD_PING                0x24
#define CMD_PONG                0x26
#define CMD_NOCOMPLETE          0x36
#define CMD_NOCOMPLETE_ALIAS1   0x56
#define CMD_NOCOMPLETE_ALIAS2   0x64
#define CMD_NORX                0x9c
#define CMD_NORX_ACK            0x9e
#define CMD_CONNECT_COMPLETE    0x66
#define CMD_CONNECT_COMPLETE_ACK 0x68
#define CMD_ADVERTISING         0xfc
#define CMD_DISCONNECT          0xf4
#define CMD_SLAVE_ACK           0xf8
#define CMD_ASSERT_MASTER       0xfa

#define CMD_WALKER_RESET_REQ    0x2a    // recv this from game
#define CMD_WALKER_RESET_RSP    0x2c
#define CMD_WALK_END_REQ        0x4e
#define CMD_WALK_END_ACK        0x50

#define CMD_EVENT_MAP           0xc0
#define CMD_EVENT_POKEMON       0xc2
#define CMD_EVENT_ITEM          0xc4
#define CMD_EVENT_ROUTE         0xc6

#define CMD_RAM_WRITE   0x06

#define EXTRA_BYTE_FROM_WALKER  0x01
#define EXTRA_BYTE_TO_WALKER    0x02

typedef enum {
    COMM_STATE_IDLE,
    COMM_STATE_KEYEX,
    COMM_STATE_READY,
    COMM_STATE_ADVERTISING,
    COMM_STATE_COUNT,
} comm_state_t;

typedef enum {
    IR_OK,
    IR_ERR_GENERAL,
    IR_ERR_UNEXPECTED_PACKET,
    IR_ERR_LONG_PACKET,
    IR_ERR_SHORT_PACKET,
    IR_ERR_SIZE_MISMATCH,
    IR_ERR_ADVERTISING_MAX,
    IR_ERR_TIMEOUT,
    IR_ERR_BAD_SEND,
    IR_ERR_BAD_SESSID,
    IR_ERR_BAD_CHECKSUM,
    IR_ERR_COUNT,
} ir_err_t;

typedef enum {
    CONNECT_STATUS_AWAITING,
    CONNECT_STATUS_DISCONNECTED,
    CONNECT_STATUS_MASTER,
    CONNECT_STATUS_SLAVE,
} connect_status_t;;

extern const char* const PW_IR_ERR_NAMES[];

void pw_ir_setup();
ir_err_t pw_ir_recv_packet();
ir_err_t pw_ir_send_packet();
comm_state_t pw_ir_get_state();
void pw_ir_set_state(comm_state_t s);

void pw_ir_set_connect_status(connect_status_t s);
connect_status_t pw_ir_get_connect_status();

uint16_t pw_ir_checksum(uint8_t packet[], size_t len);
uint16_t pw_ir_checksum_seeded(uint8_t packet[], size_t len, uint16_t seed);

ir_err_t pw_ir_listen_for_handshake();
//ir_err_t ir_handle_interaction();
//ir_err_t ir_comm_idle();
//ir_err_t ir_comm_keyex();
//ir_err_t ir_comm_advertising();
//void ir_post_keyex();




#endif /* IR_COMMS_H */

