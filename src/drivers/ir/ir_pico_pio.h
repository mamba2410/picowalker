#ifndef DRIVER_IR_PIO_H
#define DRIVER_IR_PIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Include board-specific overrides if available
#ifdef BOARD_RESOURCES_H
#include "board_resources.h"
#elif __has_include("board_resources.h")
#include "board_resources.h"
#endif

#ifndef IR_SD_PIN
#define IR_SD_PIN  9    // GP9, phys 12
#endif
#ifndef IR_PIO_RX  
#define IR_PIO_RX 10    // GP10, phys 14
#endif
#ifndef IR_PIO_TX
#define IR_PIO_TX 11    // GP11, phys 15
#endif

#define PIN_IRDA_OUT IR_PIO_TX
#define PIN_IRDA_IN  IR_PIO_RX

#define PW_IR_PIO_FRAME_ERROR_BIT (1<<15)

#endif /* DRIVER_IR_PIO_H */
