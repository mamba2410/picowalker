#ifndef DRIVER_SSD1327_H
#define DRIVER_SSD1327_H

#include <stddef.h>
#include <stdint.h>
#include "hardware/i2c.h"

#include "../screen.h"


#define OLED_ADDR   0x3c // from datasheet 8.1.5
#define OLED_WIDTH	128
#define OLED_HEIGHT	128
#define OLED_PAGE_HEIGHT 8
#define OLED_NUM_PAGES	OLED_HEIGHT/OLED_PAGE_HEIGHT
#define OLED_BPP    4
#define OLED_MAX_MEM    (OLED_WIDTH*OLED_HEIGHT) // enough to hold 1byte per pixel
#define OLED_MSG_BUF_SIZE   (OLED_MAX_MEM*OLED_BPP/8 + 1) // enough to hold a full pixel buffer

#define OLED_READ_MODE	_u(0xff)
#define OLED_WRITE_MODE	_u(0xfe)

#define OLED_CMD_OFF	0xae
#define OLED_CMD_ON	    0xaf
#define OLED_CMD_REMAP	0xa0
#define OLED_CMD_DISPLAY_MODE	0xa4
#define OLED_CMD_DATA   0x40

#define OLED_DEFAULT_MAP	0x51
#define OLED_DISPLAY_ALL_ON	0x01
#define OLED_DISPLAY_ALL_OFF	0x02
#define OLED_DISPLAY_INVERT	0x03

//#define OLED_SDA    8   // GP8, phys 11
//#define OLED_SCL    9   // GP9, phys 12
//#define OLED_SDA    4   // GP4, phys 6
//#define OLED_SCL    5   // GP5, phys 7
#define OLED_SDA    16   // GP16, phys 21
#define OLED_SCL    17   // GP17, phys 22


typedef struct {
	i2c_inst_t* i2c;
	uint speed;
	uint8_t sda;
	uint8_t scl;
    size_t width;
    size_t height;
} ssd1327_t;

typedef struct {
    size_t height;
    size_t width;
    size_t x, y;
    uint8_t *data;
    size_t size;
} oled_img_t;


int oled_write(ssd1327_t *oled, uint8_t *data, size_t len);
int oled_draw(ssd1327_t *oled, oled_img_t *img);
int oled_clear_ram(ssd1327_t *oled);
uint8_t oled_convert_colour(uint8_t c);
void oled_draw_box(ssd1327_t *oled, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t colour);

void pw_img_to_oled(pw_img_t *pw_img, oled_img_t *oled_img);

#endif /* DRIVER_SSD1327_H */

