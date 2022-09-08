#include <stdlib.h>

#include "drivers/ssd1327.h"
#include "screen.h"

static screen_t screen;

int pw_screen_init() {

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
    oled_img.data = 0;

    pw_img_to_oled(img, &oled_img);
    oled_img.x = x + screen.offset_x;
    oled_img.y = y + screen.offset_y;

    oled_draw(&(screen.chip), &oled_img);

    free(oled_img.data);

}


void pw_screen_clear() {
    oled_clear_ram(&screen.chip);
}
