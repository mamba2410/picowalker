#ifndef PW_SCREEN_H
#define PW_SCREEN_H

#include <stdint.h>

#include "drivers/ssd1327.h"

#define SCREEN_WIDTH    96
#define SCREEN_HEIGHT   64

typedef struct {
    ssd1327_t chip;
    size_t width, height;
    size_t true_width, true_height;
    size_t offset_x, offset_y;
} screen_t;


int pw_screen_init();
int pw_screen_draw_img(pw_img_t *img, size_t x, size_t y);
int pw_clear_screen();

#endif /* PW_SCREEN_H */
