#include <stdbool.h>
#include <stdint.h>

#include "hardware/gpio.h"
#include "pico/types.h"

#include "pico_buttons.h"
#include "../buttons.h"

void pw_pico_setup_buttons() {
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_LEFT, GPIO_IRQ_EDGE_FALL, true, &pw_pico_button_callback);
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_MIDDLE, GPIO_IRQ_EDGE_FALL, true, &pw_pico_button_callback);
	gpio_set_irq_enabled_with_callback(PIN_BUTTON_RIGHT, GPIO_IRQ_EDGE_FALL, true, &pw_pico_button_callback);
}

void pw_pico_button_callback(uint gp, uint32_t events) {
    uint8_t b = 0;

	switch(gp) {
		case PIN_BUTTON_LEFT:   b |= BUTTON_L; break;
		case PIN_BUTTON_MIDDLE: b |= BUTTON_M; break;
		case PIN_BUTTON_RIGHT:  b |= BUTTON_R; break;
		default: break;
	}

    pw_button_callback(b);

}
