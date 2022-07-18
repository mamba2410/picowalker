#ifndef DRIVER_SSD1327_H
#define DRIVER_SSD1327_H

#define OLED_ADDRESS	_u(0x3c) // from datasheet 8.1.5
#define OLED_WIDTH	128
#define OLED_HEIGHT	128
#define OLED_PAGE_HEIGHT 8
#define OLED_NUM_PAGES	OLED_HEIGHT/OLED_PAGE_HEIGHT

#define OLED_READ_MODE	_u(0x01)
#define OLED_WRITE_MODE	_u(0x00)

#define OLED_CMD_OFF	0xae
#define OLED_CMD_ON	0xaf
#define OLED_CMD_REMAP	0xa0
#define OLED_CMD_DISPLAY_MODE	0xa4

#define OLED_DEFAULT_MAP	0x51
#define OLED_DISPLAY_ALL_ON	0x01
#define OLED_DISPLAY_ALL_OFF	0x02
#define OLED_DISPLAY_INVERT	0x03


typedef struct {
	i2c_inst_t i2c;
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

typedef struct {
    size_t height, width;
    uint8_t *data;
    size_t size;
} pw_img_t;

int oled_write(ssd1327_t *oled, uint8_t *data, size_t len);
int oled_init(ssd1327_t *oled);
int oled_draw(ssd1327_t *oled, oled_img_t *img);

void pw_img_to_oled(pw_img_t *pw_img, oled_img_t *oled_img);

#endif /* DRIVER_SSD1327_H */

