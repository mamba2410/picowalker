#ifndef DRIVER_IR_PIO_H
#define DRIVER_IR_PIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define IR_SD_PIN  9    // GP9, phys 12
#define IR_PIO_RX 10    // GP10, phys 14
#define IR_PIO_TX 11    // GP11, phys 15

#define PIN_IRDA_OUT IR_PIO_TX
#define PIN_IRDA_IN  IR_PIO_RX

#define PW_IR_PIO_FRAME_ERROR_BIT (1<<15)

#endif /* DRIVER_IR_PIO_H */
