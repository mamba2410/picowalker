#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <hardware/uart.h>
#include <hardware/gpio.h>
#include "pico/time.h"

#include "ir_pico_pio.h"


int pw_ir_read(uint8_t *buf, size_t max_len) {
    // TODO: replace with PIO code
    return 0;
}


int pw_ir_write(uint8_t *buf, size_t len) {
    // TODO: replace with PIO code
    return 0;
}

void pw_ir_init() {
    // TODO: replace with PIO code
}

void pw_ir_clear_rx() {
    // TODO: replace with PIO code or remove
}

void pw_ir_deinit() {
    /* TODO: power saving (IR shutdown, stop uart1, etc.) */
}

