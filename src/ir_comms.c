#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "drivers/ir.h"
#include "ir_comms.h"

static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_buf[TX_BUF_LEN];
static uint8_t rx_buf_aa[RX_BUF_LEN];
static uint8_t tx_buf_aa[TX_BUF_LEN];
static uint8_t session_id[4] = {0, 0, 0, 0};

static bool peer_master = false;
static size_t advertising_attempts = 0;
static connect_status_t g_connect_status = CONNECT_STATUS_DISCONNECTED;
static comm_state_t g_comm_state = COMM_STATE_IDLE;

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
};


ir_err_t pw_ir_send_packet(uint8_t packet[], size_t len) {

    for(uint8_t i = 0; i < 4; i++)
        packet[4+i] = session_id[i];

    packet[0x02] = 0;
    packet[0x03] = 0;

	uint16_t chk = pw_ir_checksum(packet, len);

    // little-endian
	packet[0x02] = (uint8_t)(chk&0xff);
	packet[0x03] = (uint8_t)(chk>>8);


	for(size_t i = 0; i < len; i++)
		tx_buf_aa[i] = packet[i] ^ 0xaa;

    int n_written = pw_ir_write(tx_buf_aa, len);

    if(n_written != len)
        return IR_ERR_BAD_SEND;

	return IR_OK;
}


ir_err_t pw_ir_recv_packet(uint8_t packet[], size_t len) {

    int n_read = pw_ir_read(rx_buf_aa, len);

    if( ((int)n_read) != len )
        return IR_ERR_SIZE_MISMATCH;

    for(size_t i = 0; i < len; i++)
        packet[i] = rx_buf_aa[i] ^ 0xaa;


    uint8_t packet_chk_h = packet[0x03];
    uint8_t packet_chk_l = packet[0x02];
    uint16_t packet_chk = ((uint16_t)packet_chk_h << 8) | ((uint16_t)packet_chk_l);

    packet[0x02] = 0;
    packet[0x03] = 0;
    uint16_t chk = pw_ir_checksum(packet, len);
    packet[0x02] = packet_chk_l;
    packet[0x03] = packet_chk_h;

    if(packet_chk != chk && n_read>1)
        return IR_ERR_BAD_CHECKSUM;

    bool session_id_ok = true;
    for(size_t i = 0; (i<4) && session_id_ok; i++)
        session_id_ok = (packet[4+i] == session_id[i]);
    if(!session_id_ok)
        return IR_ERR_BAD_SESSID;

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

    crc = pw_ir_checksum_seeded(packet, 8, 0x0002);
    if(len>8) {
        crc = pw_ir_checksum_seeded(packet+8, len-8, crc);
    }

    return crc;
}


comm_state_t pw_ir_get_state() {
	return g_comm_state;
}

void pw_ir_set_state(comm_state_t s) {
	g_comm_state = s;
}


void pw_ir_setup() {
    pw_ir_init();
}

ir_err_t pw_ir_listen_for_handshake() {
    uint8_t *tx = tx_buf;
    uint8_t *rx = rx_buf;
    ir_err_t err;

    err = pw_ir_recv_packet(rx, 1);
    //usleep(5*1000);

    if(err != IR_OK && err != IR_ERR_BAD_SESSID && err != IR_ERR_BAD_CHECKSUM) {
        return err;
    }

    if(rx[0] != CMD_ADVERTISING)
        return IR_ERR_UNEXPECTED_PACKET;

    tx[0] = CMD_ASSERT_MASTER;
    tx[1] = EXTRA_BYTE_FROM_WALKER;
    for(size_t i = 0; i < 4; i++) {
        tx[4+i] = session_id[i];
    }

    err = pw_ir_send_packet(tx, 8);
    if(err != IR_OK) return err;

    //usleep(5000);
    size_t i = 0;
    do {
        err = pw_ir_recv_packet(rx, 8);
        i++;
    } while(rx[0] == 0xfc && i<10); // debug to clear rxbuf

    //usleep(5000);

    if(err != IR_OK) {
        printf("Error recv packet: %02x: %s\n", err, PW_IR_ERR_NAMES[err]);
        printf("Packet header: ");
        for(size_t i = 0; i < 8; i++) {
            printf("0x%02x ", rx[i]);
        }
        printf("\n");

        return err;
    }

    if(rx[0] == CMD_ASSERT_MASTER) {
        printf("Remote is master\n");
        pw_ir_set_connect_status(CONNECT_STATUS_SLAVE);
        //TODO: handle this
    } else {
        pw_ir_set_connect_status(CONNECT_STATUS_MASTER);
    }

    if(rx[0] != CMD_SLAVE_ACK) {
        printf("Error: got resp: %02x\n", tx[0]);
        pw_ir_set_connect_status(CONNECT_STATUS_DISCONNECTED);
        return IR_ERR_UNEXPECTED_PACKET;
    }


    printf("Keyex successful!\nsession_id: ");
    for(size_t i = 0; i < 4; i++) {
        session_id[i] ^= rx[4+i];
        printf("%02x", session_id[i]);
    }
    printf("\n");


    return IR_OK;
}

void pw_ir_set_connect_status(connect_status_t s) {
    g_connect_status = s;
}

connect_status_t pw_ir_get_connect_status() {
    return g_connect_status;
}

/*
uint16_t crc16_ccitt(uint8_t packet[], size_t packetSize) {
    uint16_t crc = 0;
    while(packetSize >0) {
      packetSize--;
      crc ^= (uint16_t) *packet++ << 8;
      for(size_t i = 7; i; i--) {
        if(crc & 0x8000)  crc = crc << 1 ^ 0x1021;
        else              crc = crc << 1;
      }
    }
    return crc;
}
*/

