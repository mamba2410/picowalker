#ifndef PW_DRIVER_FLASH_RP2XXX_INTERNAL_H
#define PW_DRIVER_FLASH_RP2XXX_INTERNAL_H

#define DRIVER_FLASH_IMAGES_SIZE 492

#ifndef __ASSEMBLER__
#include <stdint.h>

// Picowalker
#include "picowalker-defs.h"

extern uint8_t pw_flash_images[DRIVER_FLASH_IMAGES_SIZE];

#endif /* __ASSEMBLER__ */

#endif /* PW_DRIVER_FLASH_RP2XXX_INTERNAL_H */