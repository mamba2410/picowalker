#ifndef PW_SCREEN_PICO2_DWO_HSTX_H
#define PW_SCREEN_PICO2_DWO_HSTX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define LCD_WIDTH  240
#define LCD_HEIGHT 240
#define LCD_BYTES_PER_PIXEL 2
#define SCREEN_SCALE 1

/*
 * Set active area to be scaled up Pokewalker area, rotated by 90 degrees
 */
#define LCD_ACTIVE_WIDTH  (SCREEN_HEIGHT*SCREEN_SCALE)
#define LCD_ACTIVE_HEIGHT (SCREEN_WIDTH*SCREEN_SCALE)

#define LCD_X_OFFSET ((LCD_WIDTH-(LCD_ACTIVE_WIDTH))/2)
#define LCD_Y_OFFSET ((LCD_HEIGHT-(LCD_ACTIVE_HEIGHT))/2)

/*
 * (96*4)*(64*4)*2 = 192kiB
 * (96*2)*(64*2)*2 = 96kiB
 * 240*240*2 = 112.5kiB
 *
 * rp2040 has 256 kiB ram, so full screen buffer could be used.
 */
#define LCD_BUFFER_SIZE (112*1024 + 512)
//#define LCD_BUFFER_SIZE (LCD_ACTIVE_WIDTH*LCD_ACTIVE_HEIGHT*LCD_BYTES_PER_PIXEL)


const uint16_t colour_map[4] = {
    0xe75b, // white
    0xbe16, // light grey
    0x7c0e, // dark grey
    0x5289, // black
};


/*
 * https://files.waveshare.com/wiki/common/GC9A01A.pdf
 * Section 6 for list of commands (page 91)
 * Note the need for DCX line
 */
enum screen_cmd {
};

typedef struct lcd_s {
    int offset_x;
    int offset_y;

    size_t true_width;
    size_t true_height;
} lcd_t;

typedef struct screen_area_s {
    int x;
    int y;
    int width;
    int height;
} screen_area_t;

#define PIN_LCD_DCX     8   // GP8
#define PIN_LCD_CS      9
#define PIN_LCD_CLK     10
#define PIN_LCD_MOSI    11
#define PIN_LCD_MISO    12
#define PIN_LCD_RSI     13
#define PIN_LCD_BL      25

#endif /* PW_SCREEN_PICO2_DWO_HSTX_H */
