
#include <stdio.h>
#include <hardware/gpio.h>

#include "buttons.h"
#include "states.h"

void setup_buttons() {
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_LEFT, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_MIDDLE, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_RIGHT, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
}

void pw_irq_callback(uint gp, uint32_t events) {
	switch(gp) {
		case PIN_BUTTON_LEFT: button_pressed(BUTTON_L); break;
		case PIN_BUTTON_MIDDLE: button_pressed(BUTTON_M); break;
		case PIN_BUTTON_RIGHT: button_pressed(BUTTON_R); break;
		default: break;
	}

}

void button_pressed(uint8_t button) {

	pw_state_t state = pw_get_state();
	state_handle_button_press(state, button);

}

