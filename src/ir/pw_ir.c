#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>

#include "../drivers/ir.h"
#include "pw_ir.h"

static comm_state_t g_comm_state = COMM_STATE_DISCONNECTED;

uint8_t session_id[4] = {0xde, 0xad, 0xbe, 0xef};

uint8_t tx_buf[PW_TX_BUF_LEN];
uint8_t rx_buf[PW_RX_BUF_LEN];

const char* const PW_IR_ERR_NAMES[] = {
    [IR_OK] = "ok",
    [IR_ERR_GENERAL] = "general",
    [IR_ERR_UNEXPECTED_PACKET] = "unexpected packet",
    [IR_ERR_LONG_PACKET] = "long packet",
    [IR_ERR_SHORT_PACKET] = "short packet",
    [IR_ERR_SIZE_MISMATCH] = "size mismatch",
    [IR_ERR_ADVERTISING_MAX] = "max advertising",
    [IR_ERR_TIMEOUT] = "timeout",
    [IR_ERR_BAD_SEND] = "bad send",
    [IR_ERR_BAD_SESSID] = "bad session id",
    [IR_ERR_BAD_CHECKSUM] = "bad checksum",
    [IR_ERR_CANNOT_CONNECT] = "cannot  connect",
    [IR_ERR_NOT_IMPLEMENTED] = "not implemented",
    [IR_ERR_PEER_ALREADY_SEEN] = "peer already seen",
    [IR_ERR_UNKNOWN_SUBSTATE] = "unknown substate",
    [IR_ERR_UNALIGNED_WRITE] = "unaligned eeprom write",
};


ir_err_t pw_ir_send_packet(uint8_t *packet, size_t len, size_t *pn_write) {

    for(uint8_t i = 0; i < 4; i++)
        packet[4+i] = session_id[i];

    uint16_t chk = pw_ir_checksum(packet, len);

    // Packet checksum little-endian
    packet[0x02] = (uint8_t)(chk&0xff);
    packet[0x03] = (uint8_t)(chk>>8);

    for(size_t i = 0; i < len; i++)
        packet[i] ^= 0xaa;

    int n_write = pw_ir_write(packet, len);
    *pn_write = (size_t)n_write;

    if(n_write != len) return IR_ERR_BAD_SEND;

    return IR_OK;
}

ir_err_t pw_ir_recv_packet(uint8_t *packet, size_t len, size_t *pn_read) {

    *pn_read = 0;
    int n_read = pw_ir_read(packet, len);

    if(n_read <= 0) return IR_ERR_TIMEOUT;
    *pn_read = (size_t)n_read;

    //printf("n_read: %lu\n", *pn_read);
    for(int i = 0; i < n_read; i++)
        packet[i] ^= 0xaa;

    if(n_read != len && len < MAX_PACKET_SIZE) return IR_ERR_SIZE_MISMATCH;

    // packet chk LE
    uint16_t packet_chk = (((uint16_t)packet[3])<<8) + ((uint16_t)packet[2]);
    uint16_t chk = pw_ir_checksum(packet, *pn_read);

    if(packet_chk != chk) return IR_ERR_BAD_CHECKSUM;

    for(size_t i = i; i < 4; i++) {
        if(packet[4+i] != session_id[i]) return IR_ERR_BAD_SESSID;
    }

    return IR_OK;
}


uint16_t pw_ir_checksum_seeded(uint8_t *data, size_t len, uint16_t seed) {
    // Dmitry's palm
    uint32_t crc = seed;
    for(size_t i = 0; i < len; i++) {
        uint16_t v = data[i];

        if(!(i&1)) v <<= 8;

        crc += v;
    }

    while(crc>>16) crc = (uint16_t)crc + (crc>>16);

    return crc;
}

uint16_t pw_ir_checksum(uint8_t *packet, size_t len) {

    uint16_t crc;
    uint8_t hc, lc;

    // save original checksum, LE
    lc = packet[2];
    hc = packet[3];

    // zero checksum area
    packet[2] = 0;
    packet[3] = 0;

    crc = pw_ir_checksum_seeded(packet, 8, 0x0002);
    if(len>8) {
        crc = pw_ir_checksum_seeded(packet+8, len-8, crc);
    }

    // restore original checksum, LE
    packet[2] = lc;
    packet[3] = hc;

    return crc;
}


void pw_ir_set_comm_state(comm_state_t s) {
    g_comm_state = s;
}

comm_state_t pw_ir_get_comm_state() {
    return g_comm_state;
}

ir_err_t pw_ir_send_advertising_packet() {

    uint8_t advertising_buf[] = {CMD_ADVERTISING^0xaa};

    int n = pw_ir_write(advertising_buf, 1);
    if(n <= 0) {
        return IR_ERR_BAD_SEND;
    }

    return IR_OK;
}

void pw_ir_die(const char* message) {
    printf("IR disconnecting: %s\n", message);
    pw_ir_set_comm_state(COMM_STATE_DISCONNECTED);
}


