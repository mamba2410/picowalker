#ifndef PW_SCREEN_H
#define PW_SCREEN_H

#include <stdint.h>

#include "drivers/ssd1327.h"

#define SCREEN_WIDTH    96
#define SCREEN_HEIGHT   64

#define SCREEN_BLACK    3
#define SCREEN_DGREY    2
#define SCREEN_LGREY    1
#define SCREEN_WHITE    0

#define SCREEN_BUF_SIZE (SCREEN_WIDTH*SCREEN_HEIGHT*OLED_BPP/8)

typedef struct {
    ssd1327_t chip;
    size_t width, height;
    size_t true_width, true_height;
    size_t offset_x, offset_y;
} screen_t;


int pw_screen_init();
int pw_screen_draw_img(pw_img_t *img, size_t x, size_t y);
//int pw_screen_draw_from_eeprom(uint16_t addr, size_t len, uint8_t w, uint8_t h, uint8_t x, uint8_t y);
int pw_screen_draw_from_eeprom(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t addr, size_t len);
void pw_screen_draw_integer(uint32_t n, size_t right_x, size_t y);
void pw_screen_draw_time(uint8_t hour, uint8_t minute, uint8_t second, size_t x, size_t y);
void pw_screen_draw_subtime(uint8_t n, size_t x, size_t y, bool draw_colon);
void pw_screen_clear_area(size_t x, size_t y, size_t width, size_t height);
void pw_screen_draw_horiz_line(uint8_t x, uint8_t y, uint8_t len, uint8_t colour);

void pw_screen_clear();

#endif /* PW_SCREEN_H */
