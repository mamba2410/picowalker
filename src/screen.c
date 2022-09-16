#include <stdlib.h>
#include <string.h>

#include "drivers/ssd1327.h"
#include "screen.h"

static screen_t screen;

static uint8_t *screen_buf = 0;

int pw_screen_init() {

    if(!screen_buf)
        screen_buf = malloc(SCREEN_BUF_SIZE);

	ssd1327_t oled = {
			i2c: i2c_default,
			speed: 400*1000,
			sda: PICO_DEFAULT_I2C_SDA_PIN, // GP4
			scl: PICO_DEFAULT_I2C_SCL_PIN, // GP5
            width: OLED_WIDTH,
            height: OLED_HEIGHT,
	};
    oled_init(&oled);

    screen = (screen_t){
        chip: oled,
        width: SCREEN_WIDTH,
        height: SCREEN_HEIGHT,
        true_width: OLED_WIDTH,
        true_height: OLED_HEIGHT,
        offset_x: 0,
        offset_y: 0,
    };

    screen.offset_x = (screen.true_width-screen.width)/2;
    screen.offset_y = (screen.true_height-screen.height)/2;

}


int pw_screen_draw_img(pw_img_t *img, size_t x, size_t y) {
    oled_img_t oled_img;
    oled_img.data = screen_buf;

    pw_img_to_oled(img, &oled_img);
    oled_img.x = x + screen.offset_x;
    oled_img.y = y + screen.offset_y;

    oled_draw(&(screen.chip), &oled_img);

}


void pw_screen_clear() {
    oled_clear_ram(&screen.chip);
}


void pw_screen_clear_area(size_t x, size_t y, size_t width, size_t height) {
    size_t size = width * height/2;

    oled_img_t area = {
        x: x+screen.offset_x,
        y: y+screen.offset_y,
        width: width,
        height: height,
        size: size,
        data: screen_buf,
    };

    memset(screen_buf, 0, size);

    oled_draw(&(screen.chip), &area);

}


void pw_screen_draw_integer(uint32_t n, size_t right_x, size_t y) {

    size_t x = right_x;
    uint32_t m = n;
    for(uint32_t pos = 1; pos < n; pos*=10) {
        size_t idx = m%10;
        m = m/10;
        pw_screen_draw_img(&text_characters[idx], x, y);
        x -= 8;
    }
}

void pw_screen_draw_time(uint8_t hour, uint8_t minute, uint8_t second, size_t x, size_t y) {
    pw_screen_draw_subtime(hour, x, y, true);
    x += 48;
    pw_screen_draw_subtime(minute, x, y, true);
    x += 48;
    pw_screen_draw_subtime(second, x, y, false);
}

void pw_screen_draw_subtime(uint8_t n, size_t x, size_t y, bool draw_colon) {
    uint8_t idx;

    idx = n/10;
    pw_screen_draw_img(&text_characters[idx], x, y);
    x += 8;
    idx = n%10;
    pw_screen_draw_img(&text_characters[idx], x, y);
    if(draw_colon) {
        x += 8;
        pw_screen_draw_img(&text_character_colon, x, y);
    }
}

