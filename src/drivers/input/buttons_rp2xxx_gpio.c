#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/types.h"
#include "pico/stdlib.h"

#include "board_resources.h"
#include "../../picowalker_structures.h"
#include "../../picowalker_core.h"
#include "buttons_rp2xxx_gpio.h"
#include "../interrupts/rp2xxx_gpio.h"
#include "../sleep/dormant_rp2xxx.h"

bool acknowledge_button_presses;
uint64_t last_pressed = 0;

void pw_button_init() {

    acknowledge_button_presses = true;

    // enable pins
    gpio_init(BUTTON_LEFT_PIN);
    gpio_init(BUTTON_MIDDLE_PIN);
    gpio_init(BUTTON_RIGHT_PIN);

    // Internal pull-up
    gpio_pull_up(BUTTON_LEFT_PIN);
    gpio_pull_up(BUTTON_MIDDLE_PIN);
    gpio_pull_up(BUTTON_RIGHT_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_LEFT_PIN,   GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_MIDDLE_PIN, GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_RIGHT_PIN,  GPIO_IRQ_EDGE_FALL, true, &pw_gpio_interrupt_handler);
}

void pw_pico_button_callback(uint gp, uint32_t events) {
    (void)events;
    uint8_t b = 0;


    // Stop buttons doing anything during sleep
    if(!acknowledge_button_presses) return;

    switch(gp) {
    case BUTTON_LEFT_PIN:
        b |= PW_BUTTON_L;
        break;
    case BUTTON_MIDDLE_PIN:
        b |= PW_BUTTON_M;
        break;
    case BUTTON_RIGHT_PIN:
        b |= PW_BUTTON_R;
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


bool pw_button_is_pressed(pw_buttons_t b) {
    switch(b) {
        case PW_BUTTON_L: return gpio_get(BUTTON_LEFT_PIN) == 0;
        case PW_BUTTON_M: return gpio_get(BUTTON_MIDDLE_PIN) == 0;
        case PW_BUTTON_R: return gpio_get(BUTTON_RIGHT_PIN) == 0;
    }
    return false;
}

