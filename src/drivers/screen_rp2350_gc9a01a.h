#ifndef PW_SCREEN_RP2350_GC9A01A_H
#define PW_SCREEN_RP2350_GC9A01A_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define LCD_WIDTH           240
#define LCD_HEIGHT          240
#define LCD_BYTES_PER_PIXEL 2
#define SCREEN_SCALE        1
#define HORIZONTAL          0
#define VERTICAL            1

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
    CMD_NOP                = 0x00,
    CMD_READ_ID            = 0x04,
    CMD_READ_STATUS        = 0x09,
    CMD_SLEEP_MODE         = 0x10,
    CMD_SLEEP_OUT          = 0x11,
    CMD_PARTIAL_ON         = 0x12,
    CMD_NORMAL_DSP         = 0x13,
    CMD_INVERT_OFF         = 0x20,
    CMD_INVERT_ON          = 0x21,
    CMD_DISPLAY_OFF        = 0x28,
    CMD_DISPLAY_ON         = 0x29,
    CMD_COL_SET            = 0x2A,
    CMD_PAGE_SET           = 0x2B,
    CMD_WRITE_START        = 0x2C,
    CMD_PARTIAL            = 0x30,
    CMD_SCROLLING          = 0x33,
    CMD_TEARING_OFF        = 0x34,
    CMD_TEARING_ON         = 0x35,
    CMD_MEMORY_ACCESS      = 0x36,
    CMD_SCROLLING_START    = 0x37,
    CMD_IDLE_OFF           = 0x38,
    CMD_IDLE_ON            = 0x39,
    CMD_PIXEL_FORMAT       = 0x3A,
    CMD_WRITE_CONTINUE     = 0x3C,
    CMD_WRITE_TE_LINE      = 0x44,
    CMD_SCAN_LINE          = 0x45,
    CMD_SET_BRIGHTNESS     = 0x51,
    CMD_WRITE_CTRL_DSP     = 0x53,
    CMD_POWER_CONTROL7     = 0xA7,
    CMD_RBG_SIGNAL         = 0xB0,
    CMD_BLANKING_PORCH     = 0xB5,
    CMD_DISPLAY_FUNCTION   = 0xB6,
    CMD_TEARING_EFFECT     = 0xBA,
    CMD_POWER_CONTROL1     = 0xC1,
    CMD_POWER_CONTROL2     = 0xC3,
    CMD_POWER_CONTROL3     = 0xC4,
    CMD_POWER_CONTROL4     = 0xC9,
    CMD_READ_ID1           = 0xDA,
    CMD_READ_ID2           = 0xDB,
    CMD_READ_ID3           = 0xDC,
    CMD_INTER_REG_ENABLE2  = 0xEF,
    CMD_INTERFACE_CONTROL  = 0xF6,
    CMD_FRAME_RATE         = 0xE8,
    CMD_SPI_2DATA_CONTROL  = 0xE9,
    CMD_INTER_REG_ENALBE1  = 0xFE,
    CMD_SET_GAMMA1         = 0xF0,
    CMD_SET_GAMMA2         = 0xF1,
    CMD_SET_GAMMA3         = 0xF2,
    CMD_SET_GAMMA4         = 0xF3,
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

typedef struct lcd_attributes_s
{
    uint16_t HEIGHT;
    uint16_t WIDTH;
    uint8_t SCAN_DIRECTION;

} lcd_attributes_t

/*
 Development Board Configuration from RP2040-LCD-1.28 & RP2350-LCD-1.28
 /RP*-LCD-1.28/c/lib/DEV_CONFIG.h
*/
#define LCD_SPI_PORT spi1
#define LCD_PIN_DC   8
#define LCD_PIN_CS   9
#define LCD_PIN_CLK  10
#define LCD_PIN_MOSI 11
#define LCD_PIN_MISO 12
#define LCD_PIN_RST  13
#define LCD_PIN_BL   25

#endif /* PW_SCREEN_RP2350_GC9A01A_H */
