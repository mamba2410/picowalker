#ifndef PW_IMAGES_H
#define PW_IMAGES_H

#include <stddef.h>
#include <stdint.h>
#include "pwroms.h"

typedef struct {
    size_t height, width;
    uint8_t *data;
    size_t size;
} pw_img_t;


extern pw_img_t text_mm_dowsing;
extern pw_img_t text_mm_pokeradar;
extern pw_img_t text_mm_connect;
extern pw_img_t text_mm_trainer;
extern pw_img_t text_mm_inventory;
extern pw_img_t text_mm_settings;

#endif /* PW_IMAGES_H */
