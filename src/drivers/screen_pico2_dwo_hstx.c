#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <pico/stdlib.h>

#include "hardware/gpio.h"
#include "hardware/resets.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/hstx_ctrl.h"
#include "hardware/structs/hstx_fifo.h"

#include "../picowalker-defs.h"
#include "screen_pico2_dwo_hstx.h"

static amoled_t amoled = {0};

static uint8_t amoled_buffer[AMOLED_BUFFER_SIZE] = {0};

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

static void hstx_configure_1wire() {
    hstx_ctrl_hw->csr = 0; // Disable and reset HSTX CSR

    /*
     * Set up clock and SD0 for 1 wire MSB transmission
     * Disable pins SD1..3
     */
    hstx_ctrl_hw->bit[PIN_HSTX_SCK - PIN_HSTX_START] = HSTX_CTRL_BIT0_CLK_BITS;
    hstx_ctrl_hw->bit[PIN_HSTX_SD0 - PIN_HSTX_START] = 
        (7<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (7<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD1 - PIN_HSTX_START] = 
        (31<<HSTX_CTRL_BIT0_SEL_P_LSB) | (31<<HSTX_CTRL_BIT0_SEL_P_LSB) ;
    hstx_ctrl_hw->bit[PIN_HSTX_SD2 - PIN_HSTX_START] = 
        (31<<HSTX_CTRL_BIT0_SEL_P_LSB) | (31<<HSTX_CTRL_BIT0_SEL_P_LSB) ;
    hstx_ctrl_hw->bit[PIN_HSTX_SD3 - PIN_HSTX_START] = 
        (31<<HSTX_CTRL_BIT0_SEL_P_LSB) | (31<<HSTX_CTRL_BIT0_SEL_P_LSB) ;

    hstx_ctrl_hw->csr = 
        HSTX_CTRL_CSR_EN_BITS |             // Enable HSTX
        (31<<HSTX_CTRL_CSR_SHIFT_LSB) |     // Left-shift 1 bits
        (8 <<HSTX_CTRL_CSR_N_SHIFTS_LSB) |  // Perform 8 left-shifts before exhausting
        (1 <<HSTX_CTRL_CSR_CLKDIV_LSB);     // Clock every shift

}


static void hstx_configure_4wire() {
    hstx_ctrl_hw->csr = 0; // Disable and reset HSTX CSR


    /*
     * Set up clock and SD0..3 for 4 wire MSB transmission
     * SD3 gets MSB in nibble
     */
    hstx_ctrl_hw->bit[PIN_HSTX_SCK - PIN_HSTX_START] = HSTX_CTRL_BIT0_CLK_BITS;

    hstx_ctrl_hw->bit[PIN_HSTX_SD0 - PIN_HSTX_START] = 
        (4<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (4<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD1 - PIN_HSTX_START] = 
        (5<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (5<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD2 - PIN_HSTX_START] = 
        (6<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (6<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD3 - PIN_HSTX_START] = 
        (7<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (7<<HSTX_CTRL_BIT0_SEL_N_LSB);

    /*
     * Set up HSTX shift register
     * For now, just send one byte at a time
     */
    hstx_ctrl_hw->csr = 
        HSTX_CTRL_CSR_EN_BITS |             // Enable HSTX
        (28<<HSTX_CTRL_CSR_SHIFT_LSB) |     // Left-shift 4 bits
        (2 <<HSTX_CTRL_CSR_N_SHIFTS_LSB) |  // Perform 2 left-shifts before exhausting
        (1 <<HSTX_CTRL_CSR_CLKDIV_LSB);     // Clock every shift

}


static inline void hstx_put_word(uint32_t data) {
    // wait for fifo not full
    while(hstx_fifo_hw->stat & HSTX_FIFO_STAT_FULL_BITS);

    // place single word in fifo
    hstx_fifo_hw->fifo = data;
}


static void amoled_send_1wire(uint8_t cmd, size_t len, uint8_t data[len]) {

    // can pack inst + addr into single 32-bit
    hstx_configure_1wire();
    gpio_put(PIN_HSTX_CSB, 0);
    hstx_put_word(0x02);    // 1 wire write
    hstx_put_word(0x00);
    hstx_put_word(cmd&0xff);
    hstx_put_word(0x00);

    // send args
    for(size_t i = 0; i < len; i++) {
        hstx_put_word(data[i]);
    }
    while(!(hstx_fifo_hw->stat & HSTX_FIFO_STAT_EMPTY_BITS));
    for(size_t i = 0; i < 64; i++);
    gpio_put(PIN_HSTX_CSB, 1);
}


static void amoled_send_4wire(uint8_t cmd, size_t len, uint8_t data[len]) {

    // can pack inst + addr into single 32-bit
    hstx_configure_1wire();
    gpio_put(PIN_HSTX_CSB, 0);
    hstx_put_word(0x32);    // 4 wire write
    hstx_put_word(0x00);
    hstx_put_word(cmd);
    hstx_put_word(0x00);

    hstx_configure_4wire();
    for(size_t i = 0; i < len; i++) {
        hstx_put_word(data[i]);
    }
    gpio_put(PIN_HSTX_CSB, 1);
}


static void amoled_draw_buffer(int x_start, int y_start, int width, int height,
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

    amoled_send_1wire(CMD_NOP, 0, buf); // Send NOP to say we are done
}


static void amoled_draw_block(int x_start, int y_start, int width, int height, uint16_t colour) {

    // amoled_buffer is as large as the Pokewalker render area needs to be
    // If we are doing draws larger than this area, we need to know to do it in chunks
    size_t n_bytes = width*height*AMOLED_BYTES_PER_PIXEL;
    if(n_bytes > AMOLED_BUFFER_SIZE) {
        printf("Error: Drawing %lu bytes from undersized (%lu) buffer\n",
                n_bytes, AMOLED_BUFFER_SIZE);
        return;
    }

    // Set colour
    for(size_t i = 0; i < n_bytes; i++) {
        ((uint16_t*)amoled_buffer)[i] = colour;
    }

    amoled_draw_buffer(x_start, y_start, width, height, AMOLED_BUFFER_SIZE, amoled_buffer);
}


static void decode_img(pw_img_t *pw_img, size_t out_len, size_t out_buf[out_len]) {
    
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
            col = pw_img->height - y_normal;
            row = x_normal;

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

static screen_area_t transform_pw_to_amoled(screen_area_t pw_area, amoled_t a) {
    screen_area_t amoled_area = {0};
    amoled_area.x = (SCREEN_HEIGHT - pw_area.height - pw_area.y)*SCREEN_SCALE + a.offset_x;
    amoled_area.y = (pw_area.x)*SCREEN_SCALE + a.offset_y;
    amoled_area.width = pw_area.height * SCREEN_SCALE;
    amoled_area.height = pw_area.width * SCREEN_SCALE;
    return amoled_area;
}


static void amoled_clear_screen() {

    // Do it in two blocks since buffer isn't as large
    amoled_draw_block(0, 0, AMOLED_WIDTH, AMOLED_HEIGHT/2, 0);
    amoled_draw_block(0, AMOLED_HEIGHT/2, AMOLED_WIDTH, AMOLED_HEIGHT/2, 0);

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
    gpio_init(PIN_HSTX_CSB);
    gpio_init(PIN_SCREEN_PWREN);
    gpio_init(PIN_SCREEN_RST);
    gpio_set_dir(PIN_HSTX_CSB, GPIO_OUT);
    gpio_set_dir(PIN_SCREEN_PWREN, GPIO_OUT);
    gpio_set_dir(PIN_SCREEN_RST, GPIO_OUT);
    gpio_put(PIN_HSTX_CSB, 1);
    gpio_put(PIN_SCREEN_RST, 0);
    gpio_put(PIN_SCREEN_PWREN, 0);

    gpio_set_function(PIN_HSTX_SCK, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD0, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD1, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD2, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD3, 0/*GPIO_FUNC_HSTX*/);

    sleep_ms(5);


    /*
     * Set up clocks to 48MHz
     */
    reset_block(RESETS_RESET_HSTX_BITS);
    hw_write_masked(
        &clocks_hw->clk[clk_hstx].ctrl,
        CLOCKS_CLK_HSTX_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB << CLOCKS_CLK_HSTX_CTRL_AUXSRC_LSB,
        CLOCKS_CLK_HSTX_CTRL_AUXSRC_BITS
    );
    hw_write_masked(
        &clocks_hw->clk[clk_hstx].div,
        0x00 << CLOCKS_CLK_HSTX_DIV_INT_LSB,
        CLOCKS_CLK_HSTX_DIV_INT_BITS
    );
    unreset_block_wait(RESETS_RESET_HSTX_BITS);


    /*
     * Screen initialise sequence
     */
    //gpio_put(PIN_SCREEN_PWREN, 1);
    //sleep_ms(10);
    gpio_put(PIN_SCREEN_RST, 0);
    sleep_ms(10);
    gpio_put(PIN_SCREEN_RST, 1);
    sleep_ms(150);

    params[0] = 0x00;
    amoled_send_1wire(CMD_SLEEP_OUT, 0, params);
    sleep_ms(150);

    //while(1) {
        //amoled_send_1wire(CMD_ALL_OFF, 0, params);
        //sleep_ms(500);
    //}

    params[0] = 0xd5;
    amoled_send_1wire(CMD_PIXEL_FORMAT, 1, params);

    params[0] = 0x20;
    amoled_send_1wire(CMD_WRITE_CTRL_DSP1, 1, params);
    sleep_ms(10);

    params[0] = 0x00;
    amoled_send_1wire(CMD_SET_BRIGHTNESS, 1, params);
    sleep_ms(10);

    amoled_send_1wire(CMD_DISPLAY_ON, 0, params);
    sleep_ms(10);

    params[0] = 0xff;
    amoled_send_1wire(CMD_SET_BRIGHTNESS, 1, params);
    sleep_ms(10);

    amoled_send_1wire(CMD_ALL_ON, 0, params);
    sleep_ms(500);

    //amoled_clear_screen();

    amoled.true_width = AMOLED_WIDTH;
    amoled.true_height = AMOLED_HEIGHT;
    amoled.offset_x = (AMOLED_WIDTH-SCREEN_SCALE*SCREEN_HEIGHT)/2;
    amoled.offset_y = (AMOLED_HEIGHT-SCREEN_SCALE*SCREEN_WIDTH)/2;

    //amoled_send_1wire(CMD_NOP, 0, params);

    //amoled_draw_block(amoled.offset_x, amoled.offset_y,
    //    SCREEN_SCALE*SCREEN_HEIGHT, SCREEN_SCALE*SCREEN_WIDTH,
    //    colour_map[SCREEN_BLACK]
    //);
}


void pw_screen_draw_img(pw_img_t *img, screen_pos_t x, screen_pos_t y) {

}


void pw_screen_clear_area(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_pos_t h) {

}

void pw_screen_draw_horiz_line(screen_pos_t x, screen_pos_t y, screen_pos_t w, screen_colour_t c) {

}


void pw_screen_draw_text_box(screen_pos_t x, screen_pos_t y, 
                             screen_pos_t w, screen_pos_t h,
                             screen_colour_t c) {

}


void pw_screen_clear() {

}


void pw_screen_fill_area(screen_pos_t x, screen_pos_t y,
                         screen_pos_t w, screen_pos_t h,
                         screen_colour_t c) {

}

