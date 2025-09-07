#ifndef PW_PICO_FLASH_INTERNAL_H
#define PW_PICO_FLASH_INTERNAL_H

#define DRIVER_FLASH_IMAGES_SIZE 492

#ifndef __ASSEMBLER__
#include <stdint.h>

#include "../picowalker-defs.h"

extern uint8_t pw_flash_images[DRIVER_FLASH_IMAGES_SIZE];

#endif /* __ASSEMBLER__ */

#endif /* PW_PICO_FLASH_INTERNAL_H */
