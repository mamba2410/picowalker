#ifndef PW_SCREEN_PICO2_DWO_PIO_H
#define PW_SCREEN_PICO2_DWO_PIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define AMOLED_WIDTH 368
#define AMOLED_HEIGHT 448
#define AMOLED_BYTES_PER_PIXEL 2
#define SCREEN_SCALE 4

/*
 * Set active area to be scaled up Pokewalker area, rotated by 90 degrees
 * In AMOLED coordinates
 */
#define AMOLED_ACTIVE_WIDTH (SCREEN_HEIGHT*SCREEN_SCALE)
#define AMOLED_ACTIVE_HEIGHT (SCREEN_WIDTH*SCREEN_SCALE)

#define AMOLED_X_OFFSET ((AMOLED_WIDTH-(AMOLED_ACTIVE_WIDTH))/2)
#define AMOLED_Y_OFFSET ((AMOLED_HEIGHT-(AMOLED_ACTIVE_HEIGHT))/2)

/*
 * (96*4)*(64*4)*2 = 192kiB
 * 448*368*2 = 322kiB
 *
 * pico2 has 512kiB ram so we can fit the whole 322kiB (16bpp) or even 483kiB
 * (24bpp) buffers in here with a good amount of room to spare.
 */
#define AMOLED_BUFFER_SIZE (192*1024)
//#define AMOLED_BUFFER_SIZE (AMOLED_ACTIVE_WIDTH*AMOLED_ACTIVE_HEIGHT*AMOLED_BYTES_PER_PIXEL)


/*
// Normal
const uint16_t colour_map[4] = {
    0xe75b, // white
    0xbe16, // light grey
    0x7c0e, // dark grey
    0x5289, // black
};
*/

// Reversed nibbles
const uint16_t colour_map[4] = {
    0x7ead, // white
    0xd786, // light grey
    0xe307, // dark grey
    0xa419, // black
};

enum screen_cmd {
    CMD_NOP             = 0x00,
    CMD_SWRST           = 0x01,
    CMD_READ_ID         = 0x04,
    CMD_READ_SELFTEST   = 0x0f,
    CMD_SLEEP_OUT       = 0x11,
    CMD_PARTIAL_ON      = 0x12,
    CMD_NORMAL_DSP      = 0x13,
    CMD_INVERT_OFF      = 0x20,
    CMD_INVERT_ON       = 0x21,
    CMD_ALL_OFF         = 0x22,
    CMD_ALL_ON          = 0x23,
    CMD_DISPLAY_OFF     = 0x28,
    CMD_DISPLAY_ON      = 0x29,
    CMD_COL_SET         = 0x2a,
    CMD_PAGE_SET        = 0x2b,
    CMD_WRITE_START     = 0x2c,
    CMD_PARTIAL_ROW_SET = 0x30,
    CMD_PARTIAL_COL_SET = 0x31,
    CMD_TE_OFF          = 0x34,
    CMD_TE_ON           = 0x35,
    CMD_IDLE_OFF        = 0x38,
    CMD_IDLE_ON         = 0x39,
    CMD_PIXEL_FORMAT    = 0x3a,
    CMD_WRITE_CONTINUE  = 0x3c,
    CMD_WRITE_TE_LINE   = 0x44,
    CMD_DSTB_CTRL       = 0x4f,
    CMD_SET_BRIGHTNESS  = 0x51,
    CMD_WRITE_CTRL_DSP1 = 0x53,
    CMD_WRITE_CTRL_DPS2 = 0x55,
    CMD_SPI_MODE_CTRL   = 0xc4,
};

typedef struct amoled_s {
    int offset_x;
    int offset_y;
    size_t true_width;
    size_t true_height;
} amoled_t;

typedef struct screen_area_s {
    int x;
    int y;
    int width;
    int height;
} screen_area_t;


#define PIN_PIO_SCK    8
#define PIN_PIO_SD_START 9
#define PIN_PIO_SD0    12
#define PIN_PIO_SD1    11
#define PIN_PIO_SD2    10
#define PIN_PIO_SD3    9
#define PIN_PIO_CSB    7

#define PIN_SCREEN_RST  6
#define PIN_SCREEN_PWREN    5

#endif /* PW_SCREEN_PICO2_DWO_HSTX_H */