#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1327.h"

static uint8_t greyscale_map[] = {0x0, 0x4, 0x8, 0xF};

int oled_write(ssd1327_t *oled, uint8_t *buf, size_t len) {
	i2c_write_blocking(oled->i2c, (OLED_ADDR & OLED_WRITE_MODE), buf, len, true);
}

#define OLED_BUF_SIZE 32
int oled_init(ssd1327_t *oled) {
	uint8_t buf[OLED_BUF_SIZE];
	size_t cursor = 0;
    memset(buf, 0, OLED_BUF_SIZE);

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
    for(size_t i = 0; i < 3; i++) {
	    oled_write(oled, buf, cursor);
        sleep_ms(500);
        buf[cursor-1] ^= 1;
	    oled_write(oled, buf, cursor);
        sleep_ms(500);
    }

    oled_clear_ram(oled);
}


void oled_set_cursor(ssd1327_t *oled, oled_img_t *img) {
    uint8_t buf[8];

    size_t bc = 0;
    buf[bc++] = 0x00;
    buf[bc++] = 0x15; // column address
    buf[bc++] = img->x/2;
    buf[bc++] = (img->x + img->width)/2 - 1;
    buf[bc++] = 0x75; // row address
    buf[bc++] = img->y;
    buf[bc++] = (img->y + img->height) - 1;

    oled_write(oled, buf, bc);
}

int oled_clear_ram(ssd1327_t *oled) {
    // size in bytes, should be 8k for 128x128 4bpp oled
    size_t oled_size = oled->width * oled->height * OLED_BPP/8;

    uint8_t *buf = malloc(oled_size+1);
    memset(buf, 0, oled_size+1);

    oled_img_t img = {
        width: oled->width,
        height: oled->height,
        x: 0,
        y: 0,
        data: buf,
        size: oled_size,
    };

    int err = oled_draw(oled, &img);
    free(buf);
    return err;
}

int oled_draw(ssd1327_t *oled, oled_img_t *img) {
    uint8_t *buf = malloc(img->size+1);

    buf[0] = OLED_CMD_DATA;
    memcpy(buf+1, img->data, img->size);

    oled_set_cursor(oled, img);
    oled_write(oled, buf, img->size+1);

    free(buf);
    return 0;
}


/*
 * Convert 2bpp Pokewalker image format to 4bpp OLED format
 * Need on-the-fly decoding because game sends images
 * Adapted from pw_lcd repo
 */
void pw_img_to_oled(pw_img_t *pw_img, oled_img_t *oled_img) {
	uint8_t pixel_value, bit_plane_upper, bit_plane_lower;
	size_t row, col;

	//size_t pw_len = img_width*img_height *2/8; // 2 bytes = 8 pixels
    pw_img->size = pw_img->width * pw_img->height * 2/8;
    oled_img->width = pw_img->width;
    oled_img->height = pw_img->height;
    oled_img->size = oled_img->width * oled_img->height * OLED_BPP/8;
    uint8_t *buf = malloc(oled_img->width * oled_img->height);
    if(!oled_img->data) {
        oled_img->data = malloc(oled_img->size);
    }

	// i = number of bytes into pw_img
	for(size_t i = 0; i < pw_img->size; i += 2) {
		bit_plane_upper = pw_img->data[i];
		bit_plane_lower = pw_img->data[i+1];

		// j = index of pixel in chunk
		for(size_t j = 0; j < 8; j++) {
			// PW raw pixel value
			pixel_value  = ((bit_plane_upper>>j) & 1) << 1;
			pixel_value |= ((bit_plane_lower>>j) & 1);

			// TODO: simplify maths to go from pw_img coords to oled coords
			col = (i/2)%pw_img->width;
			row = 8*(i/(2*pw_img->width)) + j;

			// Convert pw 2bpp to oled 4bpp via map
			// assuming 1 byte per pixel
			//oled_img->data[ (row*oled_img->width)+col ] = greyscale_map[pixel_value];
			buf[ (row*oled_img->width)+col ] = greyscale_map[pixel_value];

		}
	}

    for(size_t i = 0; i < oled_img->size; i++) {
        uint8_t v = (buf[2*i]<<4) | (buf[2*i+1]&0x0f);  // merge 2 pixels into one byte
        oled_img->data[i] = v;
    }

    //free(buf);

}


