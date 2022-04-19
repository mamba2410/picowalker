#ifndef IR_COMMS_H
#define IR_COMMS_H

#include <hardware/uart.h>

#include "ir_commands.h"

#define IR_UART_ID			uart1
#define IR_UART_TX_PIN		11
#define IR_UART_RX_PIN		12
#define IR_UART_BAUD_RATE	115200
#define IR_UART_DATA_BITS	8
#define IR_UART_STOP_BITS	1
#define IR_UART_PARITY		UART_PARITY_NONE



#define RX_BUF_LEN	(128+PACKET_HEADER_LEN)
#define MAX_ADVERTISING_ATTEMPTS 20

typedef enum {
	COMM_IDLE,			// No current state
	COMM_KEYEX,			// Exchanging keys
	COMM_READY,			// 
	COMM_ADVERTISING,	// 
	COMM_STATE_COUNT,
} ir_state_t;


typedef enum {
	IR_OK,
	IR_ERR_GENERAL,
	IR_ERR_UNEXPECTED_PACKET,
	IR_ERR_LONG_PACKET,
	IR_ERR_SHORT_PACKET,
	IR_ERR_SIZE_MISMATCH,
	IR_ERR_ADVERTISING_MAX,
	IR_ERR_TIMEOUT,
	IR_ERR_COUNT,
} ir_err_t;

void setup_ir_uart();
void ir_uart_rx_interrupt();
ir_err_t ir_recv_packet();
ir_err_t ir_handle_interaction();
ir_err_t ir_comm_idle();
ir_err_t ir_comm_keyex();
ir_err_t ir_comm_advertising();
void ir_post_keyex();

ir_state_t ir_get_state();
void ir_set_state(ir_state_t s);

#endif /* IR_COMMS_H */

