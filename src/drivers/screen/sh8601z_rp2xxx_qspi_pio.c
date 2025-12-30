#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"

#include "board_resources.h"
#include "../../picowalker_structures.h"
#include "sh8601z_rp2xxx_qspi_pio.h"

#include "qspi.pio.h"

static amoled_t amoled = {0};

static uint8_t amoled_buffer[AMOLED_BUFFER_SIZE] = {0};

static struct {
    PIO pio;
    uint sm;
    uint qspi_4wire_offset;
    uint qspi_1wire_offset;
    bool is_4wire; // true for 4 wire
} pio_config;

/*
 * DWO screen QSPI interface:
 *
 * All are MSB-first
 * Pin SD3 gets MSB of nibble
 *
 * Register write:
 * - instruction 8 bits, 1 wire
 * - address 24 bits, 1 wire
 * - data variable length, 1 wire
 *
 * Pixel write:
 * - instruction 8 bits, 1 wire
 * - address 8 bits, 1 wire
 * - pixel data variable length, 4 wires
 *
 * Room for optimisation:
 *
 * - Can pack instructoin and address into single 32-bit register
 * - Can send each *pixel* at a time by writing 16-bit to fifo
 *      (DMA friendly)
 * - Can further send more data by packing two of the same pixel into the fifo
 *      (halves buffer size, still can DMA)
 * - Maybe expander can help with decoding RLE images
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


screen_area_t transform_pw_to_amoled(screen_area_t pw_area, amoled_t a) {
    screen_area_t amoled_area = {0};
    //amoled_area.x = (PW_SCREEN_HEIGHT - pw_area.height - pw_area.y)*SCREEN_SCALE + a.offset_x;
    amoled_area.x = pw_area.y * SCREEN_SCALE + a.offset_x;
    amoled_area.y = (PW_SCREEN_WIDTH - pw_area.x - pw_area.width)*SCREEN_SCALE + a.offset_y;
    amoled_area.width = pw_area.height * SCREEN_SCALE;
    amoled_area.height = pw_area.width * SCREEN_SCALE;
    return amoled_area;
}


void pio_configure_1wire() {
    pio_qspi_1wire_tx_init(
        pio_config.pio, pio_config.sm, pio_config.qspi_1wire_offset,
        8,
        //37.5f,
        1.0f,
        SCREEN_SCK_PIN,
        SCREEN_SD0_PIN
    );
    gpio_deinit(SCREEN_SD1_PIN);
    gpio_deinit(SCREEN_SD2_PIN);
    gpio_deinit(SCREEN_SD3_PIN);
    pio_config.is_4wire = false;
}

void pio_configure_4wire() {

    // Clock the machine at clk_sys/clkdiv but also account for clock taking
    // 4 cycles, so SPI clock rate is clk_sys/(4*clkdiv)
    pio_qspi_4wire_tx_init(
        pio_config.pio, pio_config.sm, pio_config.qspi_4wire_offset,
        8, // 8 bit words
        //37.5f,
        1.0f,
        SCREEN_SCK_PIN,
        SCREEN_PIO_SD_START_PIN
    );
    pio_config.is_4wire = true;
}

void __time_critical_func(pio_put_word)(uint8_t word) {

    // Doesn't work?
    //pio_sm_put_blocking(pio_config.pio, pio_config.sm, word);

    io_rw_8 *txfifo = (io_rw_8*)&pio_config.pio->txf[pio_config.sm];
    while(pio_sm_is_tx_fifo_full(pio_config.pio, pio_config.sm));
    *txfifo = word;
}

void __time_critical_func(amoled_send_1wire)(uint8_t cmd, size_t len, uint8_t data[len]) {

    // can pack inst + addr into single 32-bit
    pio_configure_1wire();
    gpio_put(SCREEN_CSB_PIN, 0);
    pio_put_word(0x02);    // 1 wire write
    pio_put_word(0x00);
    pio_put_word(cmd&0xff);
    pio_put_word(0x00);

    // send args
    for(size_t i = 0; i < len; i++) {
        pio_put_word(data[i]);
    }

    while(!pio_sm_is_tx_fifo_empty(pio_config.pio, pio_config.sm));
    sleep_us(10);

    gpio_put(SCREEN_CSB_PIN, 1);
}


void __time_critical_func(amoled_send_4wire)(uint8_t cmd, size_t len, uint8_t data[len]) {

    // can pack inst + addr into single 32-bit
    pio_configure_1wire();
    gpio_put(SCREEN_CSB_PIN, 0);
    pio_put_word(0x32);    // 4 wire write
    pio_put_word(0x00);
    pio_put_word(cmd&0xff);
    pio_put_word(0x00);

    while(!pio_sm_is_tx_fifo_empty(pio_config.pio, pio_config.sm));
    sleep_us(10);

    pio_configure_4wire();
    for(size_t i = 0; i < len; i++) {
        pio_put_word(data[i]);
    }

    while(!pio_sm_is_tx_fifo_empty(pio_config.pio, pio_config.sm));
    sleep_us(10);

    gpio_put(SCREEN_CSB_PIN, 1);
}


void amoled_draw_buffer(int x_start, int y_start, int width, int height,
        size_t len, uint8_t buf[len]) {

    uint8_t params[5] = {0};
    int x_end = x_start + width  - 1;
    int y_end = y_start + height - 1;

    params[0] = x_start >> 8;
    params[1] = x_start & 0xff;
    params[2] = x_end >> 8;
    params[3] = x_end & 0xff;
    amoled_send_1wire(CMD_COL_SET, 4, params);

    params[0] = y_start >> 8;
    params[1] = y_start & 0xff;
    params[2] = y_end >> 8;
    params[3] = y_end & 0xff;
    amoled_send_1wire(CMD_PAGE_SET, 4, params);

    size_t n_bytes = width*height*AMOLED_BYTES_PER_PIXEL;
    if(n_bytes > len) {
        printf("Error: Drawing %lu bytes from undersized (%lu) buffer\n",
                n_bytes, len);
        return;
    }

    amoled_send_4wire(CMD_WRITE_START, n_bytes, buf);       // First send
    amoled_send_4wire(CMD_WRITE_CONTINUE, n_bytes, buf);    // Second send
    //amoled_send_4wire(CMD_WRITE_START,    len, buf);       // First send
    //amoled_send_4wire(CMD_WRITE_CONTINUE, len, buf);    // Second send

    amoled_send_1wire(CMD_NOP, 0, buf); // Send NOP to say we are done
}


void amoled_draw_block(int x_start, int y_start, int width, int height, uint16_t colour) {

    // amoled_buffer is as large as the Pokewalker render area needs to be
    // If we are doing draws larger than this area, we need to know to do it in chunks
    size_t n_bytes = width*height*AMOLED_BYTES_PER_PIXEL;
    if(n_bytes > AMOLED_BUFFER_SIZE) {
        printf("Error: Drawing %lu bytes from undersized (%lu) buffer\n",
                n_bytes, AMOLED_BUFFER_SIZE);
        return;
    }

    // Set colour
    for(size_t i = 0; i < n_bytes; i+=2) {
        amoled_buffer[i+0] = colour>>8;
        amoled_buffer[i+1] = colour&0xff;
    }

    amoled_draw_buffer(x_start, y_start, width, height, AMOLED_BUFFER_SIZE, amoled_buffer);
    //amoled_draw_buffer(x_start, y_start, width, height, n_bytes, amoled_buffer);
}




void amoled_clear_screen() {

    uint16_t colour = 0x0000;
    // Do it in two blocks since buffer isn't as large
    for(size_t i = 0; i < 2; i++){
        amoled_draw_block(0, i*AMOLED_HEIGHT/2, AMOLED_WIDTH, AMOLED_HEIGHT/2, colour);
    }

}

void amoled_reset() {
    uint8_t params[4] = {0};

    /*
     * Screen initialise sequence
     */
    gpio_put(SCREEN_RST_PIN, 0);
    sleep_ms(3);
    gpio_put(SCREEN_RST_PIN, 1);
    sleep_ms(50);

    params[0] = 0x00;
    amoled_send_1wire(CMD_SLEEP_OUT, 0, params);
    sleep_ms(150);

    params[0] = 0xd5;
    //params[0] = 0x55;
    amoled_send_1wire(CMD_PIXEL_FORMAT, 1, params);
    sleep_ms(10);

    params[0] = 0x20;
    amoled_send_1wire(CMD_WRITE_CTRL_DSP1, 1, params);
    sleep_ms(25);

    params[0] = 0x00;
    amoled_send_1wire(CMD_SET_BRIGHTNESS, 1, params);
    sleep_ms(10);

    params[0] = 0x7f;
    amoled_send_1wire(CMD_SET_BRIGHTNESS, 1, params);
    sleep_ms(10);

    amoled_clear_screen();

    amoled.true_width = AMOLED_WIDTH;
    amoled.true_height = AMOLED_HEIGHT;
    amoled.offset_x = AMOLED_X_OFFSET;
    amoled.offset_y = AMOLED_Y_OFFSET;

    amoled_draw_block(
        amoled.offset_x, amoled.offset_y,
        AMOLED_ACTIVE_WIDTH, AMOLED_ACTIVE_HEIGHT,
        colour_map[PW_SCREEN_BLACK]
    );

    // Note: different final row/column to draw areas
    params[0] = (AMOLED_X_OFFSET)>>8;
    params[1] = (AMOLED_X_OFFSET)&0xff;
    params[2] = (AMOLED_X_OFFSET + AMOLED_ACTIVE_WIDTH)>>8;
    params[3] = (AMOLED_X_OFFSET + AMOLED_ACTIVE_WIDTH)&0xff;
    amoled_send_1wire(CMD_PARTIAL_COL_SET, 4, params);

    params[0] = (AMOLED_Y_OFFSET)>>8;
    params[1] = (AMOLED_Y_OFFSET)&0xff;
    params[2] = (AMOLED_Y_OFFSET + AMOLED_ACTIVE_HEIGHT)>>8;
    params[3] = (AMOLED_Y_OFFSET + AMOLED_ACTIVE_HEIGHT)&0xff;
    amoled_send_1wire(CMD_PARTIAL_ROW_SET, 4, params);

    amoled_send_1wire(CMD_PARTIAL_ON, 0, params);

    amoled_send_1wire(CMD_DISPLAY_ON, 0, params);
}


/*
 * ============================================================================
 * Driver functions
 * ============================================================================
 */

void pw_screen_init() {

    uint8_t params[4] = {0};

    /*
     * Set up HSTX
     * Adapted from https://github.com/raspberrypi/pico-examples/blob/master/hstx/spi_lcd/hstx_spi_lcd.c
     * From SPI 1-lane to 4
     */

    /*
     * Set up manual CSB
     */
    gpio_init(SCREEN_CSB_PIN);
    gpio_init(SCREEN_PWREN_PIN);
    gpio_init(SCREEN_RST_PIN);
    gpio_set_dir(SCREEN_CSB_PIN, GPIO_OUT);
    gpio_set_dir(SCREEN_PWREN_PIN, GPIO_OUT);
    gpio_set_dir(SCREEN_RST_PIN, GPIO_OUT);
    gpio_put(SCREEN_CSB_PIN, 1);
    gpio_put(SCREEN_PWREN_PIN, 1);
    gpio_put(SCREEN_RST_PIN, 0);

    pio_config.pio = SCREEN_PIO_HW;
    pio_config.sm = SCREEN_PIO_SM;
    pio_config.qspi_4wire_offset = pio_add_program(pio_config.pio, &qspi_4wire_tx_cpha0_program);
    pio_config.qspi_1wire_offset = pio_add_program(pio_config.pio, &qspi_1wire_tx_cpha0_program);

    // Each SPI clock is 4 PIO cycles

    amoled_reset();
}


void pw_screen_draw_img(pw_img_t *img, pw_screen_pos_t x, pw_screen_pos_t y) {
    // TODO: checks image isn't too large
    decode_img(img, AMOLED_BUFFER_SIZE, amoled_buffer);

    // Put decoded, transformed image in `amoled_buffer`
    decode_img(img, AMOLED_BUFFER_SIZE, amoled_buffer);

    // Transform image area to amoled coordinates
    screen_area_t amoled_area = transform_pw_to_amoled((screen_area_t){
        .x = x,
        .y = y,
        .width = img->width,
        .height = img->height,
    }, amoled);
    amoled_draw_buffer(
            //((PW_SCREEN_HEIGHT-img->height-y)*SCREEN_SCALE)+amoled.offset_x, (x*SCREEN_SCALE)+amoled.offset_y,
            //img->height*SCREEN_SCALE, img->width*SCREEN_SCALE,
            amoled_area.x, amoled_area.y,
            amoled_area.width, amoled_area.height,
            2*amoled_area.width*amoled_area.height,
            amoled_buffer);
}


void pw_screen_clear_area(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t w, pw_screen_pos_t h) {

    screen_area_t amoled_area = transform_pw_to_amoled((screen_area_t){
        .x = x,
        .y = y,
        .width = w,
        .height = h,
    }, amoled);

    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[PW_SCREEN_WHITE]
    );

}

void pw_screen_draw_horiz_line(pw_screen_pos_t x, pw_screen_pos_t y, pw_screen_pos_t w, pw_screen_color_t c) {
    screen_area_t amoled_area = transform_pw_to_amoled((screen_area_t){
        .x = x,
        .y = y,
        .width = w,
        .height = 1,
    }, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[c]);

}


void pw_screen_draw_text_box(pw_screen_pos_t x1, pw_screen_pos_t y1,
                             pw_screen_pos_t width, pw_screen_pos_t height,
                             pw_screen_color_t c) {

    // assume y2 > y1 and x2 > x1
    pw_screen_pos_t x2 = x1 + width - 1;
    pw_screen_pos_t y2 = y1 + height - 1;

    screen_area_t amoled_area = {0}, pw_area = {0};

    // top bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = width, .height = 1};
    amoled_area = transform_pw_to_amoled(pw_area, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[c]);

    // bottom bar
    pw_area = (screen_area_t){.x = x1, .y = y2, .width = width, .height = 1};
    amoled_area = transform_pw_to_amoled(pw_area, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[c]);

    // left bar
    pw_area = (screen_area_t){.x = x1, .y = y1, .width = 1, .height = height};
    amoled_area = transform_pw_to_amoled(pw_area, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[c]);

    // right bar
    pw_area = (screen_area_t){.x = x2, .y = y1, .width = 1, .height = height};
    amoled_area = transform_pw_to_amoled(pw_area, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[c]);

}


void pw_screen_clear() {
    screen_area_t amoled_area = transform_pw_to_amoled((screen_area_t){
        .x=0, .y=0,
        .width=PW_SCREEN_WIDTH, .height=PW_SCREEN_HEIGHT
    }, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[PW_SCREEN_WHITE]
    );

}


void pw_screen_fill_area(pw_screen_pos_t x, pw_screen_pos_t y,
                         pw_screen_pos_t w, pw_screen_pos_t h,
                         pw_screen_color_t c) {
    screen_area_t amoled_area = transform_pw_to_amoled((screen_area_t){.x=x, .y=y, .width=w, .height=h}, amoled);
    amoled_draw_block(
        amoled_area.x, amoled_area.y,
        amoled_area.width, amoled_area.height,
        colour_map[c]
    );

}


void pw_screen_sleep() {
    // Enable display standby
    uint8_t params[2] = {0x01};
    amoled_send_1wire(CMD_DSTB_CTRL, 1, params);
}


void pw_screen_wake() {
    // Wake it up and re-configure it
    // Same as a power-on reset
    amoled_reset();
}


void pw_screen_set_brightness(uint8_t level) {
    if(level > PW_SCREEN_MAX_BRIGHTNESS) return;
    
    uint8_t brightness_range = AMOLED_MAX_BRIGHTNESS - AMOLED_MIN_BRIGHTNESS + 1;
    uint8_t level_range = PW_SCREEN_MAX_BRIGHTNESS + 1;
    float step = (float)brightness_range / (float)level_range;
    uint8_t amoled_setting = AMOLED_MIN_BRIGHTNESS + (uint8_t)((float)level * step);

    amoled_send_1wire(CMD_SET_BRIGHTNESS, 1, &amoled_setting);
    printf("[Debug] Set brightness to 0x%02x (%d)\n", amoled_setting, level);
}

