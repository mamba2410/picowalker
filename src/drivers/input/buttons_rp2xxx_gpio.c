#include <stdbool.h>
#include <stdint.h>

#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/types.h"
#include "pico/stdlib.h"

#include "../picowalker-defs.h"
#include "buttons_pico.h"
#include "gpio_interrupts_pico.h"
#include "power_pico.h"

bool acknowledge_button_presses;
uint64_t last_pressed = 0;

void pw_button_init() {

    acknowledge_button_presses = true;

    // enable pins
    gpio_init(PIN_BUTTON_LEFT);
    gpio_init(PIN_BUTTON_MIDDLE);
    gpio_init(PIN_BUTTON_RIGHT);

    // Internal pull-up
    gpio_pull_up(PIN_BUTTON_LEFT);
    gpio_pull_up(PIN_BUTTON_MIDDLE);
    gpio_pull_up(PIN_BUTTON_RIGHT);

    gpio_set_irq_enabled_with_callback(PIN_BUTTON_LEFT,   GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_MIDDLE, GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_RIGHT,  GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);
}

void pw_pico_button_callback(uint gp, uint32_t events) {
    uint8_t b = 0;


    // Stop buttons doing anything during sleep
    if(!acknowledge_button_presses) return;

    switch(gp) {
    case PIN_BUTTON_LEFT:
        b |= BUTTON_L;
        break;
    case PIN_BUTTON_MIDDLE:
        b |= BUTTON_M;
        break;
    case PIN_BUTTON_RIGHT:
        b |= BUTTON_R;
        break;
    default:
        printf("[Info] Button callback on pin %d\n", gp);
        break;
    }

    uint64_t now = time_us_64();
    if( (now-last_pressed) > DEBOUNCE_TIME_US ) {
        pw_button_callback(b);
        last_pressed = now;
    }

    // Add one-shot timer to keep track of user actions
    set_user_idle_timer();

}
