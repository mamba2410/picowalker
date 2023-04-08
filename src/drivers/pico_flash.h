#ifndef PW_PICO_FLASH_H
#define PW_PICO_FLASH_H

#include <stdint.h>

#include "../flash.h"

void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf);

#endif /* PW_PICO_FLASH_H */
