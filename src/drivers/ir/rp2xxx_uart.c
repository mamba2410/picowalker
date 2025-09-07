#include <stdint.h>
#include <stddef.h>

#include <stdlib.h>
#include <stdio.h>

#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#include "rp2xxx_uart.h"


int pw_ir_read(uint8_t *buf, size_t max_len) {
    size_t cursor = 0;
    int64_t diff;

    volatile absolute_time_t start, now, last_read;
    start = get_absolute_time();
    do {
        now = get_absolute_time();
        diff = absolute_time_diff_us(start, now);
    } while( !uart_is_readable(IR_UART_ID) && diff < 50000);

    diff = 0;
    last_read = get_absolute_time();
    do {
        if(uart_is_readable(IR_UART_ID)) {
		    buf[cursor] = uart_getc(IR_UART_ID);
		    cursor++;
            last_read = get_absolute_time();
        }
        now = get_absolute_time();
        diff = absolute_time_diff_us(last_read, now); // signed difference
        //printf("%ld ", diff);
    } while( diff < 3742);

#ifdef DEBUG_IR
    printf("read: (%d)", cursor);
    for(size_t i = 0; i < cursor; i++) {
        if(i%16 == 0) printf("\n");
        if(i%i == 0)  printf(" ");
        printf("%02x", buf[i]^0xaa);
    }
    printf("\n");
#endif

    return cursor;
}


int pw_ir_write(uint8_t *buf, size_t len) {
    size_t i;
#ifdef DEBUG_IR
    printf("write: (%d)", len);
	for(i = 0; i < len; i++) {
		uart_putc_raw(IR_UART_ID, buf[i]);
        if(i%16 == 0) printf("\n");
        if(i%i == 0)  printf(" ");
        printf("%02x", buf[i]^0xaa);
	}
    printf("\n");
#else
	for(i = 0; i < len; i++) {
		uart_putc_raw(IR_UART_ID, buf[i]);
	}
#endif

    return i;
}

void pw_ir_init() {
	// Initialise uart for IR comms
	uart_init(IR_UART_ID, IR_UART_BAUD_RATE);
	gpio_set_function(IR_UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(IR_UART_RX_PIN, GPIO_FUNC_UART);

	//int __unused actual_baudrate = uart_set_baudrate(IR_UART_ID, IR_UART_BAUD_RATE);
	uart_set_hw_flow(IR_UART_ID, false, false);
	uart_set_format(IR_UART_ID, IR_UART_DATA_BITS, IR_UART_STOP_BITS, IR_UART_PARITY);
	uart_set_fifo_enabled(IR_UART_ID, true);
    uart_set_translate_crlf(IR_UART_ID, false);

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

void pw_ir_deinit() {
    /* TODO: power saving (IR shutdown, stop uart1, etc.) */
}

