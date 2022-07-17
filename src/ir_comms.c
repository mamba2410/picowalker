#include <stdint.h>
#include <stdio.h>
#include <pico/stdlib.h>
//#include <pico/mem.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include <string.h>

#include "ir_comms.h"

//static uint64_t last_recv = 0;
static ir_state_t ir_state = COMM_IDLE;
static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_buf[RX_BUF_LEN];
static size_t rx_cursor = 0;
static size_t tx_cursor = 0;
static uint8_t pw_key[4] = {0, 0, 0, 0};
static uint32_t session_id = 0x00000000;

static bool peer_master = false;
static size_t advertising_attempts = 0;

/*
 *	Interrupt handler on IR_UART RX
 *	TODO: Decide if we want interrupt-driven or loop-driven
 */
void ir_uart_rx_interrupt() {
	printf("Interrupt on ir uart\n");
	ir_err_t __unused err = ir_recv_packet();
}


ir_err_t ir_recv_packet() {
	rx_cursor = 0;
	while(uart_is_readable(IR_UART_ID) && (rx_cursor<RX_BUF_LEN)) {
		rx_buf[rx_cursor] = uart_getc(IR_UART_ID);
		rx_cursor++;
	}

	if(rx_cursor >= RX_BUF_LEN) {
		return IR_ERR_LONG_PACKET;
	}

	if(rx_cursor < 8) {
		return IR_ERR_SHORT_PACKET;
	}

	ir_err_t err = ir_handle_interaction();
	return err;
}


ir_err_t ir_send_packet(uint8_t packet[], size_t len) {
	if(len > RX_BUF_LEN) {
		return IR_ERR_LONG_PACKET;
	}

	uint16_t chk = checksum(packet, len);

	packet[0x02] = (uint8_t)(chk >> 8);
	packet[0x03] = (uint8_t)(chk*0xff);

	// TODO: do I even need to memcpy?
	memcpy(tx_buf, packet, len);

	for(size_t i = 0; i < len; i++) {
		tx_buf[i] ^= 0xaa;
		uart_putc_raw(IR_UART_ID, tx_buf[i]);
	}

	return IR_OK;
}

uint16_t checksum(uint8_t packet[], size_t len) {
	uint16_t chk = 0x0002;

	for(size_t i = 1; i < len; i += 2) {
		chk += packet[i];
	}

	for(size_t i = 0; i  < len; i += 2) {
		if( (packet[i] << 8) > UINT16_MAX - chk ) chk++;
		chk += packet[i] << 8;
	}

	chk = ((chk<<8) & 0xff00) | (chk) & 0x00ff;

	return chk;
}

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


/*
 * We have an interaction, what do we do with it?
 */
ir_err_t ir_handle_interaction() {

	printf("Handling IR interaction\n");
	switch (ir_state) {
		case COMM_ADVERTISING: {
								   ir_comm_advertising();
								   break;
							   }
		case COMM_IDLE: {
							ir_comm_idle();
							break;
						}

		case COMM_KEYEX: {
							 ir_comm_keyex();
							 break;
						 }
		default: {
					// TODO: Error handling
					ir_state = COMM_IDLE;
					return IR_ERR_GENERAL;
				 }
	}



	return IR_OK;
}


/*
 *	What do we do if we're idle?
 */
ir_err_t ir_comm_idle() {
	printf("Idle, attempting keyex");
	if( (rx_cursor > 0) && (rx_buf[0] == CMD_FC) ) {
		tx_buf[0] = CMD_FA;
		tx_buf[1] = 0x01;
		tx_cursor = 8;

		ir_state = COMM_KEYEX;
	} else {
		return IR_ERR_UNEXPECTED_PACKET;
	}

	// send packet

	return IR_OK;
}


ir_err_t ir_comm_keyex() {
	if(rx_cursor != 8) {
		//printf("");
		ir_state = COMM_IDLE;
		return IR_ERR_SIZE_MISMATCH;
	}

	if((rx_buf[0] != CMD_F8) || (rx_buf[1] != 0x02)) {
		ir_state = COMM_IDLE;
		return IR_ERR_UNEXPECTED_PACKET;
	}

	printf("Keyex successful\n");
	// copy key
	memcpy(&rx_buf[4], pw_key, 4);

	ir_post_keyex();

	return IR_OK;
}


// TODO: Do something
void ir_post_keyex() {

}

/*
 *	We're the walker advertising.
 *	We should throw out 0xFC and wait to see what we get.
 *	This usually gets called once per RX interrupt
 */
ir_err_t ir_comm_advertising() {

	ir_err_t err;

	uint8_t packet[8];
	const size_t len = 8;

	memset(packet, 0, len);
	packet[0x01] = 0x02;

	// If we found another walker advertising
	if( rx_buf[0] == CMD_FC ) {

		printf("Got it!\n");
		packet[0x00] = CMD_FA;	// Reply command
		packet[0x01] = 0x01;	// Declare we are the master
		// generate random session ID

		err = ir_send_packet(packet, len);

		advertising_attempts = 0;
		return err;
	}

	// What about other commands?

	// TODO: Correct advertising, don't want this in here
	for( ; advertising_attempts < MAX_ADVERTISING_ATTEMPTS; advertising_attempts++ ) {
		packet[0x00] = CMD_FC;
		err = ir_send_packet(packet, len);
	}

	if( err == IR_OK ) {
		if( advertising_attempts >= MAX_ADVERTISING_ATTEMPTS ) {
			err = IR_ERR_ADVERTISING_MAX;
		} else {
			err = IR_ERR_GENERAL;
		}
	}

	advertising_attempts = 0;
	return err;
}


ir_err_t ir_comm_peer_play() {

}


ir_state_t ir_get_state() {
	return ir_state;
}

void ir_set_state(ir_state_t s) {
	ir_state = s;
}


void setup_ir_uart() {
	// Initialise uart for IR comms
	uart_init(IR_UART_ID, 2400);
	gpio_set_function(IR_UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(IR_UART_RX_PIN, GPIO_FUNC_UART);

	int __unused actual_baudrate = uart_set_baudrate(IR_UART_ID, IR_UART_BAUD_RATE);
	uart_set_hw_flow(IR_UART_ID, false, false);
	uart_set_format(IR_UART_ID, IR_UART_DATA_BITS, IR_UART_STOP_BITS, IR_UART_PARITY);
	uart_set_fifo_enabled(IR_UART_ID, false); // TODO: Use fifos?

	// Set up uart RX interrupt
	// Listen to the correct uart irq
	int uart_irq = IR_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
	irq_set_exclusive_handler(uart_irq, ir_uart_rx_interrupt);
	irq_set_enabled(uart_irq, true);

	uart_set_irq_enables(IR_UART_ID, true, false);
}

