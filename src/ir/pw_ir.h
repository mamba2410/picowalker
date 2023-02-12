#ifndef PW_IR_H
#define PW_IR_H

#include <stdint.h>
#include <stddef.h>

#define PW_RX_BUF_LEN   256
#define PW_TX_BUF_LEN   256

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
#define CMD_WALK_START          0x5a

#define CMD_EVENT_MAP           0xc0
#define CMD_EVENT_POKEMON       0xc2
#define CMD_EVENT_ITEM          0xc4
#define CMD_EVENT_ROUTE         0xc6

#define CMD_EVENT_MAP_STAMPS           0xd0
#define CMD_EVENT_POKEMON_STAMPS       0xd2
#define CMD_EVENT_ITEM_STAMPS          0xd4
#define CMD_EVENT_ROUTE_STAMPS         0xd6

#define CMD_RAM_WRITE   0x06

#define EXTRA_BYTE_FROM_WALKER  0x01
#define EXTRA_BYTE_TO_WALKER    0x02

#define MAX_ADVERTISING_PACKETS 20

#define MAX_PACKET_SIZE (128+8)

#define PW_IR_READ_TIMEOUT_MS   200u
#define PW_IR_READ_TIMEOUT_US   (PW_IR_READ_TIMEOUT_MS*1000)
#define PW_IR_READ_TIMEOUT_DS   (PW_IR_READ_TIMEOUT_MS/100)

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
    IR_ERR_CANNOT_CONNECT,
    IR_ERR_NOT_IMPLEMENTED,
    IR_ERR_PEER_ALREADY_SEEN,
    IR_ERR_UNKNOWN_SUBSTATE,
    IR_ERR_UNALIGNED_WRITE,
    IR_ERR_COUNT,
} ir_err_t;

typedef enum {
    COMM_STATE_AWAITING,
    COMM_STATE_DISCONNECTED,
    COMM_STATE_MASTER,
    COMM_STATE_SLAVE,
} comm_state_t;


extern const char* const PW_IR_ERR_NAMES[];
extern uint8_t session_id[];
extern uint8_t tx_buf[];
extern uint8_t rx_buf[];

ir_err_t pw_ir_send_packet(uint8_t *packet, size_t len, size_t *n_read);
ir_err_t pw_ir_recv_packet(uint8_t *packet, size_t len, size_t *n_write);
ir_err_t pw_ir_send_advertising_packet();

uint16_t pw_ir_checksum_seeded(uint8_t *packet, size_t len, uint16_t seed);
uint16_t pw_ir_checksum(uint8_t *packet, size_t len);

void pw_ir_set_comm_state(comm_state_t s);
comm_state_t pw_ir_get_comm_state();
void pw_ir_die(const char* message);
void pw_ir_delay_ms(size_t ms);


#endif /* PW_IR_H */

