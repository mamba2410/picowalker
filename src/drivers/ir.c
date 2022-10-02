#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <hardware/uart.h>
#include <hardware/gpio.h>

#include "ir.h"


int pw_ir_read(uint8_t *buf, size_t max_len) {
    size_t cursor = 0;

    while( !uart_is_readable(IR_UART_ID) );

	while(uart_is_readable(IR_UART_ID) && (cursor<max_len)) {
		buf[cursor] = uart_getc(IR_UART_ID);
		cursor++;
	}

    return cursor;
}


int pw_ir_write(uint8_t *buf, size_t len) {
    size_t i;
	for(i = 0; i < len; i++) {
		uart_putc_raw(IR_UART_ID, buf[i]);
	}

    return i;
}

int pw_ir_init() {
	// Initialise uart for IR comms
	uart_init(IR_UART_ID, IR_UART_BAUD_RATE);
	gpio_set_function(IR_UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(IR_UART_RX_PIN, GPIO_FUNC_UART);

	int __unused actual_baudrate = uart_set_baudrate(IR_UART_ID, IR_UART_BAUD_RATE);
	uart_set_hw_flow(IR_UART_ID, false, false);
	uart_set_format(IR_UART_ID, IR_UART_DATA_BITS, IR_UART_STOP_BITS, IR_UART_PARITY);
	uart_set_fifo_enabled(IR_UART_ID, true);

	while(uart_is_readable(IR_UART_ID)) {
		uart_getc(IR_UART_ID);
    }


    /*
	// Set up uart RX interrupt
	// Listen to the correct uart irq
	int uart_irq = IR_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
	irq_set_exclusive_handler(uart_irq, ir_uart_rx_interrupt);
	irq_set_enabled(uart_irq, true);
	uart_set_irq_enables(IR_UART_ID, true, false);
    */
}

void pw_ir_clear_rx() {
	while(uart_is_readable(IR_UART_ID)) {
		uart_getc(IR_UART_ID);
    }
}

void pw_it_deinit() {}

