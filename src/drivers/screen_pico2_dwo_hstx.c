#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../picowalker-defs.h"
#include "screen_pico2_dwo_hstx.h"

static amoled_t amoled = {0};

void amoled_send_hstx(size_t len, uint8_t data[len]) {
    /* Reading chapter "12.11 HSTX" i rp2350 datasheet
     * HSTX has a 32-bit shift register that *rotates* the data
     * a *configurable* number of times before it moves on to the
     * next set of data.
     * In 16-bit mode, we can fill the register with 2*16 bits of 
     * pixel data, then shift 4 bits at a time, 16 shifts (64 bits)
     * which will then have outputted the data 4 times when only
     * filling with 2 bytes.
     * Note: the register rotates right, so to send MSB-first we 
     * shift right by 28 bits (equiv left rotate by 4)
     * Idea will need more refining
     *
     * To disable DDR, set BITx.SEL_N = BITx.SEL_P
     * Bits do not necessarily need to be in order on the pins, but pins
     * must be contiguous
     * There is a separate clock generator pin mode
     *
     * The command expander has a "repeat" mode which may be able to be used to
     * take the colour-indexed image data and repeat it as many times as needed
     * This is for future reference though.
     */
}

/*
 * ============================================================================
 * Driver functions
 * ============================================================================
 */

void pw_screen_init() {

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

