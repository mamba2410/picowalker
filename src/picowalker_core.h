#ifndef PICOWALKER_CORE_H
#define PICOWALKER_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "picowalker_structures.h"

/**
 * @file picowalker_core.h
 *
 * Collection of functions that the drivers can call to feed the core.
 * Mostly just callbacks, but some are utility functions as well
 *
 */

/**
 * Main setup and loop functions
 */
extern void pw_run(); // Not recommended
extern void pw_setup();
extern void (*pw_current_loop)();
extern void pw_normal_loop();
extern void pw_sleep_loop();
extern void pw_ir_loop();

/**
 * Driver module should call this function when a button interrupt fires
 */
extern void pw_button_callback(pw_buttons_t b);

/**
 * Audio period table, stored in core but needed by drivers
 */
extern const uint8_t PW_AUDIO_PERIODTAB[];

extern pw_volume_t pw_audio_get_volume();

/**
 * Get the current power mode
 */
extern int pw_power_get_mode();

#endif /* PICOWALKER_DRIVERS_H */

