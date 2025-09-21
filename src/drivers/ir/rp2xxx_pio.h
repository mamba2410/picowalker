#ifndef DRIVER_IR_PIO_H
#define DRIVER_IR_PIO_H

#include <stdint.h>
#include <stddef.h>

#include "board_resources.h"

#define PIN_IRDA_OUT IR_TX_PIN
#define PIN_IRDA_IN  IR_RX_PIN

#define PW_IR_PIO_FRAME_ERROR_BIT (1<<15)

#endif /* DRIVER_IR_PIO_H */