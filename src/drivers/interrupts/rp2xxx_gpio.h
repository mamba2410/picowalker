#ifndef PW_INTERRUPTS_PICO_H
#define PW_INTERRUPTS_PICO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void pw_gpio_interrupt_handler(uint gp, uint32_t events);

#endif /* PW_INTERRUPTS_PICO_H */
