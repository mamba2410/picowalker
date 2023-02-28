#ifndef PW_SCREEN_H
#define PW_SCREEN_H

#include <stdint.h>
#include <stddef.h>

#include "eeprom.h"

#define SCREEN_REDRAW_DELAY_US  500000

#define PW_SCREEN_WIDTH    96
#define PW_SCREEN_HEIGHT   64

#define SCREEN_BLACK    3
#define SCREEN_DGREY    2
#define SCREEN_LGREY    1
#define SCREEN_WHITE    0

typedef uint8_t screen_pos_t;       // 0-95
typedef uint8_t screen_colour_t;    // 2-bits, pw style

typedef struct {
    screen_pos_t height, width;
    uint8_t *data;
    size_t size;
} pw_img_t;

typedef struct {
    screen_pos_t width, height;
    screen_pos_t true_width, true_height;
    screen_pos_t offset_x, offset_y;
} screen_t;


/*
 *  Functions defined by the drivers
 */
extern void pw_screen_init();
extern void pw_screen_draw_img(
    pw_img_t *img,
    screen_pos_t x, screen_pos_t y
);
extern void pw_screen_clear_area(
    screen_pos_t x, screen_pos_t y,
    screen_pos_t width, screen_pos_t height
);
extern void pw_screen_draw_horiz_line(
    screen_pos_t x, screen_pos_t y,
    screen_pos_t len,
    screen_colour_t colour
);
extern void pw_screen_draw_text_box(
    screen_pos_t x1, screen_pos_t y1,
    screen_pos_t x2, screen_pos_t y2,
    screen_colour_t colour
);
extern void pw_screen_clear();

/*
 *  Derived functions
 */
void pw_screen_draw_from_eeprom(
    screen_pos_t x, screen_pos_t y,
    screen_pos_t w, screen_pos_t h,
    eeprom_addr_t addr,
    size_t len
);
void pw_screen_draw_integer(uint32_t n, size_t right_x, size_t y);
void pw_screen_draw_time(uint8_t hour, uint8_t minute, uint8_t second, size_t x, size_t y);
void pw_screen_draw_subtime(uint8_t n, size_t x, size_t y, bool draw_colon);


#endif /* PW_SCREEN_H */
