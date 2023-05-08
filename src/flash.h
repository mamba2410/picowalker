#ifndef PW_FLASH_H
#define PW_FLASH_H

#include <stdint.h>

/// @file flash.h

typedef enum {
    FLASH_IMG_POKEWALKER,
    FLASH_IMG_FACE_NEUTRAL,
    FLASH_IMG_FACE_HAPPY,
    FLASH_IMG_FACE_SAD,
    FLASH_IMG_UP_ARROW,
    FLASH_IMG_IR_ACTIVE,
    FLASH_IMG_TINY_CHARS,
} pw_flash_img_t;

extern void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf);

#endif /* PW_FLASH_H */
