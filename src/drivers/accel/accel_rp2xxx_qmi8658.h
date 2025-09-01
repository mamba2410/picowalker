#ifndef PW_DRIVER_ACCEL_RP2XXX_QMI8658_H
#define PW_DRIVER_ACCEL_RP2XXX_QMI8658_H

// Pico
#include "stdio.h"
#include "pico/stdlib.h"

// Pico WaveShare Drivers Library
#include "QMI8658.h"

// Picowalker
#include "picowalker-defs.h"

<<<<<<< HEAD
// Additional pedometer functions
void pw_accel_reset_steps();
void pw_accel_add_steps(uint32_t steps);

// Global step counter (for external access)
extern uint32_t accumulated_steps;

=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
#endif /* PW_DRIVER_ACCEL_RP2XXX_QMI8658_H */