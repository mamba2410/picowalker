#include <stdlib.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "ssd1327.h"
#include "../screen.h"

static ssd1327_t oled = {0,};
static screen_t screen = {0,};
static uint8_t greyscale_map[] = {0x0, 0x4, 0x8, 0xF};

static uint8_t *oled_decode_buf = 0;    // for decoding images into
static uint8_t *oled_image_buf = 0;    // for storing images
static uint8_t *oled_msg_buf = 0; // I2C message buffer


int oled_write(ssd1327_t *oled, uint8_t *buf, size_t len) {
	i2c_write_blocking(oled->i2c, (OLED_ADDR & OLED_WRITE_MODE), buf, len, true);
}

int oled_read(ssd1327_t *oled, uint8_t *buf, size_t len) {
	i2c_read_blocking(oled->i2c, (OLED_ADDR & OLED_READ_MODE), buf, len, true);
}

void pw_screen_init() {

    oled.width  = OLED_WIDTH;
    oled.height = OLED_HEIGHT;
    oled.i2c    = i2c0;
    oled.speed  = 400*1000;
    oled.sda    = OLED_SDA;
    oled.scl    = OLED_SCL;

    if(!oled_decode_buf)
        oled_decode_buf = malloc(OLED_MAX_MEM); // 8bpp, unpacked

    if(!oled_image_buf)
        oled_image_buf = malloc(OLED_MAX_MEM);

    if(!oled_msg_buf)
        oled_msg_buf = malloc(OLED_MSG_BUF_SIZE); // 4bpp, packed

    uint8_t *buf = oled_msg_buf;
    size_t cursor = 0;

	i2c_init(oled.i2c, oled.speed);
	gpio_set_function(oled.sda, GPIO_FUNC_I2C);
	gpio_set_function(oled.scl, GPIO_FUNC_I2C);
	gpio_pull_up(oled.sda);
	gpio_pull_up(oled.scl);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_OFF;
	oled_write(&oled, buf, cursor);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_REMAP;
	buf[cursor++] = OLED_DEFAULT_MAP;
	oled_write(&oled, buf, cursor);

	cursor = 0;
	buf[cursor++] = 0x00;
	buf[cursor++] = OLED_CMD_ON;
	oled_write(&oled, buf, cursor);

    screen.width    = SCREEN_WIDTH;
    screen.height   = SCREEN_HEIGHT;
    screen.true_width   = oled.width;
    screen.true_height   = oled.height;
    screen.offset_x = (screen.true_width-screen.width)/2;
    screen.offset_y = (screen.true_height-screen.height)/2;

    oled_clear_ram(&oled);
}


void oled_draw_box(ssd1327_t *oled, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t colour) {
    uint8_t buf[8];

    size_t bc = 0;
    buf[bc++] = 0x00;
    buf[bc++] = 0x15; // column address
    buf[bc++] = x1/2;
    buf[bc++] = x2/2;
    buf[bc++] = 0x75; // row address
    buf[bc++] = y1;
    buf[bc++] = y2;

    oled_write(oled, buf, bc);

    oled_msg_buf[0] = OLED_CMD_DATA;
    size_t len = (x2-x1+1)*(y2-y1+1);

    // if we are only 1 pixel wide, only set nibble
    if(x2 == x1) {
        if( x1%2 == 0)
            colour = colour<<4;
    } else {
        colour = colour | colour<<4;
    }

    for(size_t i = 0; i < len; i++)
        oled_msg_buf[1+i] = colour;

    oled_write(oled, oled_msg_buf, len+1);

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
    //size_t oled_size = oled->width * oled->height * OLED_BPP/8;

    memset(oled_msg_buf, 0, OLED_MSG_BUF_SIZE);

    oled_img_t img = {
        width: oled->width,
        height: oled->height,
        x: 0,
        y: 0,
        data: oled_msg_buf,
        size: OLED_MSG_BUF_SIZE,
    };

    int err = oled_draw(oled, &img);
    return err;
}

int oled_draw(ssd1327_t *oled, oled_img_t *img) {

    oled_set_cursor(oled, img);

    oled_msg_buf[0] = OLED_CMD_DATA;
    memcpy(oled_msg_buf+1, img->data, img->size);

    oled_write(oled, oled_msg_buf, img->size+1);

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
    //uint8_t *buf = malloc(oled_img->width * oled_img->height);
    uint8_t *buf = oled_decode_buf;

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


uint8_t oled_convert_colour(uint8_t c) {
    if(c < 4)
        return greyscale_map[c];
    else
        return 0xff;
}

void pw_screen_draw_img(pw_img_t *img, screen_pos_t x, screen_pos_t y) {
    oled_img_t oled_img;
    oled_img.data = oled_image_buf;

    pw_img_to_oled(img, &oled_img);
    oled_img.x = x + screen.offset_x;
    oled_img.y = y + screen.offset_y;

    oled_draw(&oled, &oled_img);

}


void pw_screen_clear() {
    oled_clear_ram(&oled);
}

void pw_screen_clear_area(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height) {
    size_t size = width * height/2;

    oled_img_t area = {
        x: x+screen.offset_x,
        y: y+screen.offset_y,
        width: width,
        height: height,
        size: size,
        data: oled_image_buf,
    };

    memset(oled_image_buf, 0, size);

    oled_draw(&oled, &area);

}


void pw_screen_draw_horiz_line(screen_pos_t x, screen_pos_t y, screen_pos_t len, screen_colour_t colour) {
    oled_img_t img = {
        x: x + screen.offset_x,
        y: y + screen.offset_y,
        width: len,
        height: 1,
        size: len/2,
        data: oled_image_buf
    };

    colour = oled_convert_colour(colour);

    for(uint8_t i = 0; i < len/2; i++) {
        oled_image_buf[i] = colour | (colour<<4);   // 2 pixels per byte
    }

    oled_draw(&oled, &img);
}


void pw_screen_draw_text_box(screen_pos_t x1, screen_pos_t y1, screen_pos_t x2, screen_pos_t y2, screen_colour_t colour) {
    x1 = x1 + screen.offset_x;
    x2 = x2 + screen.offset_x;
    y1 = y1 + screen.offset_y;
    y2 = y2 + screen.offset_y;
    oled_draw_box(&oled, x1, y1, x1, y2, oled_convert_colour(colour));
    oled_draw_box(&oled, x2, y1, x2, y2, oled_convert_colour(colour));
    oled_draw_box(&oled, x1, y1, x2, y1, oled_convert_colour(colour));
    oled_draw_box(&oled, x1, y2, x2, y2, oled_convert_colour(colour));
}
