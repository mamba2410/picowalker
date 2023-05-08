#ifndef PW_BUTTONS_H
#define PW_BUTTONS_H

#include <stdint.h>

/// @file buttons.h

//#define DEBOUNCE_TIME_US    50000   // 50ms
#define DEBOUNCE_TIME_US    100000   // 100ms

// Add an input type? For things that aren't just buttons
enum {
    BUTTON_L = 0x01,
    BUTTON_M = 0x02,
    BUTTON_R = 0x04,
};

extern void pw_button_init();

void pw_button_callback(uint8_t b);

#endif /* PW_BUTTONS_H */
