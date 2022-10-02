#ifndef DRIVER_IR_H
#define DRIVER_IR_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <hardware/uart.h>


#define IR_UART_ID			uart1
#define IR_UART_TX_PIN		11
#define IR_UART_RX_PIN		12
#define IR_UART_BAUD_RATE	115200
#define IR_UART_DATA_BITS	8
#define IR_UART_STOP_BITS	1
#define IR_UART_PARITY		UART_PARITY_NONE

int pw_ir_read(uint8_t *buf, size_t max_len);
int pw_ir_write(uint8_t *buf, size_t len);

int pw_ir_init();
void pw_ir_deinit();

void pw_ir_clear_rx();

#endif /* DRIVER_IR_H */
