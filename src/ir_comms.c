#include <stdint.h>
#include <pico/stdlib.h>
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

// Interrupt handler on IR_UART RX
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


/*
 * We have an interaction, what do we do with it?
 */
ir_err_t ir_handle_interaction() {

	printf("Handling IR interaction\n");
	switch (ir_state) {
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

