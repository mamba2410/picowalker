#ifndef PW_INTERRUPTS_PICO_H
#define PW_INTERRUPTS_PICO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void pw_gpio_interrupt_handler(uint gp, uint32_t events);

#endif /* PW_INTERRUPTS_PICO_H */

