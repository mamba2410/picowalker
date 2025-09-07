#ifndef DRIVER_IR_H
#define DRIVER_IR_H

#include <stdint.h>
#include <stddef.h>

#include <stdlib.h>

//#define DEBUG_IR

#define IR_UART_ID			uart1
//#define IR_UART_TX_PIN		4   // GP4, phys 6
//#define IR_UART_RX_PIN	    5	// GP5, phys 7
#define IR_UART_TX_PIN		8  // GP8, phys 11
#define IR_UART_RX_PIN	    9  // GP9, phys 12
#define IR_UART_BAUD_RATE	115200
#define IR_UART_DATA_BITS	8
#define IR_UART_STOP_BITS	1
#define IR_UART_PARITY		UART_PARITY_NONE

int pw_ir_read(uint8_t *buf, size_t max_len);
int pw_ir_write(uint8_t *buf, size_t len);

void pw_ir_init();
void pw_ir_deinit();

void pw_ir_clear_rx();

#endif /* DRIVER_IR_H */
