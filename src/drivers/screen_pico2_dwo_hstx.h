#ifndef PW_SCREEN_PICO2_DWO_HSTX_H
#define PW_SCREEN_PICO2_DWO_HSTX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define AMOLED_WIDTH 368
#define AMOLED_HEIGHT 448
#define AMOLED_BYTES_PER_PIXEL 2
#define SCREEN_SCALE 2

/*
 * (96*4)*(64*4)*2 = 192kiB
 * 448*368*2 = 322kiB
 *
 * pico2 has 512kiB ram so we can fit the whole 322kiB (16bpp) or even 483kiB
 * (24bpp) buffers in here with a good amount of room to spare.
 * Might be able to do some shenanegans with the HSTX to always transmit
 * each byte `SCREEN_SCALE` times to save on buffer space.
 */
//#define AMOLED_BUFFER_SIZE (192*1024)
#define AMOLED_BUFFER_SIZE (AMOLED_WIDTH*AMOLED_HEIGHT*AMOLED_BYTES_PER_PIXEL)


const uint16_t colour_map[4] = {
    0xe75b, // white
    0xbe16, // light grey
    0x7c0e, // dark grey
    0x5289, // black
};

enum screen_cmd {
    CMD_NOP             = 0x00,
    CMD_SWRST           = 0x01,
    CMD_READ_ID         = 0x04,
    CMD_READ_SELFTEST   = 0x0f,
    CMD_SLEEP_OUT       = 0x11,
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
    CMD_TE_OFF          = 0x34,
    CMD_TE_ON           = 0x35,
    CMD_IDLE_OFF        = 0x38,
    CMD_IDLE_ON         = 0x39,
    CMD_PIXEL_FORMAT    = 0x3a,
    CMD_WRITE_CONTINUE  = 0x3c,
    CMD_WRITE_TE_LINE   = 0x44,
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


#define PIN_HSTX_START  12
#define PIN_HSTX_SCK    13 // GP13, phys 17
#define PIN_HSTX_SD0    14 // GP14, phys 19
#define PIN_HSTX_SD1    15 // GP15, phys 20
#define PIN_HSTX_SD2    16 // GP16, phys 21
#define PIN_HSTX_SD3    17 // GP17, phys 22
#define PIN_HSTX_CSB    18 // GP18, phys 24

#define PIN_SCREEN_RST  22 // GP22, phys 29
#define PIN_SCREEN_PWREN    27 // GP27, phys 32
#endif /* PW_SCREEN_PICO2_DWO_HSTX_H */
