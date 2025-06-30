#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <pico/stdlib.h>

#include "hardware/gpio.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/spi.h"

//#include "hardware/structs/hstx_ctrl.h"
//#include "hardware/structs/hstx_fifo.h"

#include "../picowalker-defs.h"

static lcd_t lcd = {0};
static uint8_t lcd_buffer[LCD_BUFFER_SIZE] = {0};
static lcd_attributes_t lcd_attributes = {0};

/*
 * Screen for the Waveshare rp2350 touch LCS 1.28"
 * It's a circular screen with touch controls, but touch isn't implemented here.
 * 240x240 resolution, assume corners are cut off?
 */


static void decode_img(pw_img_t *pw_img, size_t out_len, uint8_t out_buf[out_len]) {

    uint8_t pixel_value, bpu, bpl;
    size_t row, col, stride = pw_img->height;

    pw_img->size = pw_img->width * pw_img->height * 2/8;

    // quit if the output buffer can't hold all the data
    if(out_len < pw_img->size * 2*SCREEN_SCALE*SCREEN_SCALE) {
        printf("Error: Decoded image (%lu bytes) is larger than output buffer (%lu bytes)\n",
                pw_img->size * 2*SCREEN_SCALE*SCREEN_SCALE, out_len);
        return;
    }

    // i = number of bytes into image
    for(size_t i = 0; i < pw_img->size; i+= 2) {
        bpu = pw_img->data[i+0];
        bpl = pw_img->data[i+1];

        // j = index of pixel in chunk
        for(size_t j = 0; j < 8; j++) {
            pixel_value  = ((bpu>>j) & 1) << 1;
            pixel_value |= ((bpl>>j) & 1);

            // transform coords
            size_t x_normal = (i/2)%pw_img->width;
            size_t y_normal = 8*(i/(2*pw_img->width)) + j;
            //col = pw_img->height - y_normal - 1;
            //row = x_normal;
            col = y_normal;
            row = pw_img->width - x_normal - 1;

            // now we have pixel coordinate, write to all pixels
            // that need the colour
            for(size_t py = 0; py < SCREEN_SCALE; py++) {
                for(size_t px = 0; px < SCREEN_SCALE; px++) {
                    size_t base_index = SCREEN_SCALE*((SCREEN_SCALE*row+py)*stride) + SCREEN_SCALE*col+px;

                    out_buf[2*base_index+0] = colour_map[pixel_value]>>8;
                    out_buf[2*base_index+1] = colour_map[pixel_value]&0xff;
                    if(2*base_index > out_len) {
                        printf("Error: Decode img output out of bounds\n");
                        return;
                    }
                }
            }
        }
    }

}

// TODO Adjust this code so it's only rendering 128 x 96
screen_area_t transform_pw_to_lcd(screen_area_t pw_area, lcd_t a) {
    screen_area_t lcd_area = {0};
    //lcd_area.x = (SCREEN_HEIGHT - pw_area.height - pw_area.y)*SCREEN_SCALE + a.offset_x;
    lcd_area.x = pw_area.y * SCREEN_SCALE + a.offset_x;
    lcd_area.y = (SCREEN_WIDTH + 1 - pw_area.x - pw_area.width)*SCREEN_SCALE + a.offset_y;
    lcd_area.width = pw_area.height * SCREEN_SCALE;
    lcd_area.height = pw_area.width * SCREEN_SCALE;
    return lcd_area;
}

/*
 * ============================================================================
 * Driver functions
 * ============================================================================
 */

void pw_screen_init() 
{
    // Initialize WaveShare 1.28" LCD
    LCD_1N28_Init(HORIZONTAL);
    LCD_1N28_Clear(WHITE);
    DEV_SET_PWM(100);

    // Initialize LVGL
    lv_init();

    // Initialize LVGL Display
    lv_disp_draw_buf_init(&display_buffer, buffer0, buffer1, DISP_HOR_RES * DISP_VER_RES / 2);
    lv_disp_drv_init(&display_driver);
    display_driver.draw_buf = &display_buffer;
    display_driver.flush_cb = display_flush_callback;
    display_driver.hor_res = DISP_HOR_RES;
    display_driver.ver_res = DISP_VER_RES;
    lv_disp_t *display = lv_disp_drv_register(&display_driver);
}


void pw_screen_draw_img(pw_img_t *img, screen_pos_t x, screen_pos_t y) 
{
    // TODO: checks image isn't too large
    decode_img(img, LCD_BUFFER_SIZE, lcd_buffer);

    // Put decoded, transformed image in `lcd_buffer`
    decode_img(img, LCD_BUFFER_SIZE, lcd_buffer);

    // Transform image area to lcd coordinates
    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = img->width, .height = img->height};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);

    lcd_draw_buffer(lcd_buffer, lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*lcd_draw_buffer(
            //((SCREEN_HEIGHT-img->height-y)*SCREEN_SCALE)+lcd.offset_x, (x*SCREEN_SCALE)+lcd.offset_y,
            //img->height*SCREEN_SCALE, img->width*SCREEN_SCALE,
            lcd_area.x, lcd_area.y,
            lcd_area.width, lcd_area.height,
            2*lcd_area.width*lcd_area.height,
            lcd_buffer);*/
}


void pw_screen_clear_area(screen_pos_t x, screen_pos_t y, screen_pos_t width, screen_pos_t height) 
{

    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = width, .height = height};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);

    // Clear Area via White Colour
    lcd_draw_buffer(colour_map[0], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[SCREEN_WHITE]
    );
    */
}

void pw_screen_draw_horiz_line(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_colour_t c) 
{
    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = w, .height = 1};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);
    
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */
}


void pw_screen_draw_text_box(screen_pos_t x1, screen_pos_t y1, screen_pos_t width, screen_pos_t height, screen_colour_t c) 
{

    // assume y2 > y1 and x2 > x1
    screen_pos_t x2 = x1 + width - 1;
    screen_pos_t y2 = y1 + height - 1;

    screen_area_t lcd_area = {0}, pw_area = {0};

    // top bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = width, .height = 1};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */

    // bottom bar
    pw_area = (screen_area_t){.x = x1, .y = y2, .width = width, .height = 1};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */

    // left bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = 1, .height = height};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */

    // right bar
    pw_area = (screen_area_t){.x = x2, .y = y1, .width = 1, .height = height};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);
    */
}


void pw_screen_clear() 
{
    screen_area_t pw_area = (screen_area_t){.x=0, .y=0, .width=SCREEN_WIDTH, .height=SCREEN_HEIGHT};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[0], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[SCREEN_WHITE]
    );
    */
}


void pw_screen_fill_area(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_pos_t h, screen_colour_t c) 
{
    screen_area_t pw_area = (screen_area_t){.x = x, .y = y, .width = w, .height = h};
    screen_area_t lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_buffer(colour_map[c], lcd_area.x, lcd_area.y, lcd_area.width, lcd_area.height);
    /*
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]
    );
    */
}

/********************************************************************************
Function: Turn off Display and enter Sleep Mode
Parameters:
*********************************************************************************/
void pw_screen_sleep() {
    // Enable display standby
    lcd_send_command(CMD_DISPLAY_OFF);
    sleep_ms(10);
    lcd_send_command(CMD_SLEEP_MODE);
    sleep_ms(100);
}

/********************************************************************************
Function: Removes Sleep Mode and turns on Display
Parameters:
*********************************************************************************/
void pw_screen_wake() {
    // Wake it up and re-configure it
    // Same as a power-on reset
    //lcd_reset();
    lcd_send_command(CMD_SLEEP_OUT);
    sleep_ms(100);
    lcd_send_command(CMD_DISPLAY_ON);
}
