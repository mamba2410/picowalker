#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <pico/stdlib.h>

#include "hardware/gpio.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/hstx_ctrl.h"
#include "hardware/structs/hstx_fifo.h"

#include "../picowalker-defs.h"
#include "screen_pico2_dwo_hstx.h"


static uint8_t lcd_buffer[AMOLED_BUFFER_SIZE] = {0};

/*
 * Screen for the Waveshare rp2040 touch LCS 1.28"
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


void lcd_write_data(size_t len, uint8_t data[len]) {
    
    // TODO
    // DCX goes high to indicate data being sent
    // CSB low to send data
    // Send data
    // CSB high to end transaction
    // Don't care about DCX line, next transaction can change it

}

void lcd_write_command(size_t len, uint8_t data[len]) {
    
    // TODO
    // DCX goes low to indicate command being sent
    // CSB low to send data
    // Send data
    // CSB high to end transaction
    // Don't care about DCX line, next transaction can change it

}

screen_area_t transform_pw_to_lcd(screen_area_t pw_area, lcd_t a) {
    screen_area_t lcd_area = {0};
    //lcd_area.x = (SCREEN_HEIGHT - pw_area.height - pw_area.y)*SCREEN_SCALE + a.offset_x;
    lcd_area.x = pw_area.y * SCREEN_SCALE + a.offset_x;
    lcd_area.y = (SCREEN_WIDTH + 1 - pw_area.x - pw_area.width)*SCREEN_SCALE + a.offset_y;
    lcd_area.width = pw_area.height * SCREEN_SCALE;
    lcd_area.height = pw_area.width * SCREEN_SCALE;
    return lcd_area;
}

void lcd_draw_buffer(int x_start, int y_start, int width, int height,
        size_t len, uint8_t buf[len]) {

    uint8_t params[5] = {0};
    int x_end = x_start + width  - 1;
    int y_end = y_start + height - 1;

    // TODO
}


void lcd_draw_block(int x_start, int y_start, int width, int height, uint16_t colour) {
    // Send start/end commands for x and y, with DCX pin low?
    // TODO

    // Fill local buffer with colour
    // TODO

    // Send data with DCX pin high
    // TODO
}




void lcd_clear_screen() {
    // TODO
}

void lcd_reset() {
    uint8_t params[8] = {0};

    // Send reset signal via reset pin
    // TODO

    // Screen initialise sequence
    // See `lib/LCD/LCD_1in28.c` in waveshare C demo code.
    // TODO

    // Setup screen offset
    // TODO

    // Clear screen memory by sending all zeroes
    // TODO
}


/*
 * ============================================================================
 * Driver functions
 * ============================================================================
 */

void pw_screen_init() {

    uint8_t params[4] = {0};

    /*
     * SPI 4-wire (with mosi, miso, and D/C line)
     * MSB first, D/CX should be high on LSB if its data and low on command.
     * Can "cheat" and just leave it high for the whole SPI comms if sending data.
     * RESX should stay high in normal operation
     * Run at XX MHz
     */
    // TODO: set up SPI

    // Reset and clear memory
    lcd_reset();

}


void pw_screen_draw_img(pw_img_t *img, screen_pos_t x, screen_pos_t y) {
    // TODO: checks image isn't too large
    decode_img(img, AMOLED_BUFFER_SIZE, lcd_buffer);

    // Put decoded, transformed image in `lcd_buffer`
    decode_img(img, AMOLED_BUFFER_SIZE, lcd_buffer);

    // Transform image area to lcd coordinates
    screen_area_t lcd_area = transform_pw_to_lcd((screen_area_t){
        .x = x,
        .y = y,
        .width = img->width,
        .height = img->height,
    }, lcd);
    lcd_draw_buffer(
            //((SCREEN_HEIGHT-img->height-y)*SCREEN_SCALE)+lcd.offset_x, (x*SCREEN_SCALE)+lcd.offset_y,
            //img->height*SCREEN_SCALE, img->width*SCREEN_SCALE,
            lcd_area.x, lcd_area.y,
            lcd_area.width, lcd_area.height,
            2*lcd_area.width*lcd_area.height,
            lcd_buffer);
}


void pw_screen_clear_area(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_pos_t h) {

    screen_area_t lcd_area = transform_pw_to_lcd((screen_area_t){
        .x = x,
        .y = y,
        .width = w,
        .height = h,
    }, lcd);

    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[SCREEN_WHITE]
    );

}

void pw_screen_draw_horiz_line(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_colour_t c) {
    screen_area_t lcd_area = transform_pw_to_lcd((screen_area_t){
        .x = x,
        .y = y,
        .width = w,
        .height = 1,
    }, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);

}


void pw_screen_draw_text_box(screen_pos_t x1, screen_pos_t y1,
                             screen_pos_t width, screen_pos_t height,
                             screen_colour_t c) {

    // assume y2 > y1 and x2 > x1
    screen_pos_t x2 = x1 + width - 1;
    screen_pos_t y2 = y1 + height - 1;

    screen_area_t lcd_area = {0}, pw_area = {0};

    // top bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = width, .height = 1};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);

    // bottom bar
    pw_area = (screen_area_t){.x = x1, .y = y2, .width = width, .height = 1};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);

    // left bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = 1, .height = height};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);

    // right bar
    pw_area = (screen_area_t){.x = x2, .y = y1, .width = 1, .height = height};
    lcd_area = transform_pw_to_lcd(pw_area, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]);

}


void pw_screen_clear() {
    screen_area_t lcd_area = transform_pw_to_lcd((screen_area_t){
        .x=0, .y=0,
        .width=SCREEN_WIDTH, .height=SCREEN_HEIGHT
    }, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[SCREEN_WHITE]
    );

}


void pw_screen_fill_area(screen_pos_t x, screen_pos_t y,
                         screen_pos_t w, screen_pos_t h,
                         screen_colour_t c) {
    screen_area_t lcd_area = transform_pw_to_lcd((screen_area_t){.x=x, .y=y, .width=w, .height=h}, lcd);
    lcd_draw_block(
        lcd_area.x, lcd_area.y,
        lcd_area.width, lcd_area.height,
        colour_map[c]
    );

}

void pw_screen_sleep() {
    // Enable display standby
    uint8_t params[2] = {0x01};
    lcd_send_1wire(CMD_DSTB_CTRL, 1, params);
}

void pw_screen_wake() {
    // Wake it up and re-configure it
    // Same as a power-on reset
    lcd_reset();
}
