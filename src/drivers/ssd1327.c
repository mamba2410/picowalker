#include <hardware/i2c.h>

#include "ssd1327.h"

static uint8_t greyscale_map[] = {0x0, 0x4, 0x8, 0xF};

int oled_write(ssd1327_t *oled, uint8_t *buf, size_t len) {
	i2c_write_blocking(oled->i2c, (OLED_ADDR | OLED_WRITE_MODE), buf, len, false);
}

int oled_init(ssd1327_t *oled) {
	uint8_t buf[8];
	size_t cursor = 0;

	i2c_init(oled->i2c, oled->speed);
	gpio_set_function(oled->sda, GPIO_FUNC_I2C);
	gpio_set_function(oled->scl, GPIO_FUNC_I2C);
	gpio_pull_up(oled->sda);
	gpio_pull_up(oled->scl);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_OFF;
	oled_write(oled, buf, cursor);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_REMAP;
	buf[cursor++] = OLED_DEFAULT_MAP;
	oled_write(oled, buf, cursor);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_ON;
	oled_write(oled, buf, cursor);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_DISPLAY_MODE | OLED_DISPLAY_ALL_ON;
	oled_write(oled, buf, cursor);

}


/*
 * Convert 2bpp Pokewalker image format to 4bpp OLED format
 * Need on-the-fly decoding because game sends images
 * Adapted from pw_lcd repo
 */
void pw_img_to_oled(uint8_t *pw_img, uint8_t *buf, size_t img_width, size_t img_height) {
	uint8_t pixel_value, bit_plane_upper, bit_plane_lower;
	size_t row, col;

	size_t pw_len = img_width*img_height *2/8; // 2 bytes = 8 pixels

	// i = number of bytes into pw_img
	for(size_t i = 0; i < pw_len; i += 2) {
		bit_plane_upper = pw_img[i];
		bit_plane_lower = pw_img[i+1];

		// j = index of pixel in chunk
		for(size_t j = 0; j < 8; j++) {
			// PW raw pixel value
			pixel_value  = ((bit_plane_upper>>j) & 1) << 1;
			pixel_value |= ((bit_plane_lower>>j) & 1);

			// TODO: simplify maths to go from pw_img coords to oled coords
			col = (i/2)%img_width;
			row = 8*i/(2*img_width) + j;

			// Convert pw 2bpp to oled 4bpp via map
			// assuming 1 byte per pixel
			buf[ (row*img_width)+col ] = greyscale_map[pixel_value];

		}
	}

	
}

