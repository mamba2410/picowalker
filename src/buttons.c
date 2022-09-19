
#include <stdio.h>
#include <hardware/gpio.h>

#include "buttons.h"
#include "states.h"

void pw_setup_buttons() {
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_LEFT, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_MIDDLE, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_RIGHT, GPIO_IRQ_EDGE_FALL, true, &pw_irq_callback);
}

void pw_irq_callback(uint gp, uint32_t events) {
	switch(gp) {
		case PIN_BUTTON_LEFT: pw_state_handle_input(BUTTON_L); break;
		case PIN_BUTTON_MIDDLE: pw_state_handle_input(BUTTON_M); break;
		case PIN_BUTTON_RIGHT: pw_state_handle_input(BUTTON_R); break;
		default: break;
	}

}

