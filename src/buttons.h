#ifndef PW_BUTTONS_H
#define PW_BUTTONS_H

#define PIN_BUTTON_LEFT 20
#define PIN_BUTTON_MIDDLE 19
#define PIN_BUTTON_RIGHT 18

// Add an input type? For htings that aren't just buttons
enum {
	BUTTON_L = 0x01,
	BUTTON_M = 0x02,
	BUTTON_R = 0x04,
};


void setup_buttons();
void pw_irq_callback(uint gp, uint32_t events);
void button_pressed(uint8_t button);

#endif /* PW_BUTTONS_H */
