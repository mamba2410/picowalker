#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hardware/resets.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/hstx_ctrl.h"
#include "hardware/structs/hstx_fifo.h"

#include "../picowalker-defs.h"
#include "screen_pico2_dwo_hstx.h"

static amoled_t amoled = {0};

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
 *      (not DMA friendly)
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
        (31<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (31<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD1 - PIN_HSTX_START] = 0; // Disable other bits
    hstx_ctrl_hw->bit[PIN_HSTX_SD2 - PIN_HSTX_START] = 0;
    hstx_ctrl_hw->bit[PIN_HSTX_SD3 - PIN_HSTX_START] = 0;

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
        (28<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (28<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD1 - PIN_HSTX_START] = 
        (29<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (29<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD2 - PIN_HSTX_START] = 
        (30<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (30<<HSTX_CTRL_BIT0_SEL_N_LSB);
    hstx_ctrl_hw->bit[PIN_HSTX_SD3 - PIN_HSTX_START] = 
        (31<<HSTX_CTRL_BIT0_SEL_P_LSB) | 
        (31<<HSTX_CTRL_BIT0_SEL_N_LSB);

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
    while(hstx_ctrl_hw->stat & HSTX_FIFO_STAT_FULL_BITS);

    // place single word in fifo
    hstx_fifo_hw->fifo = data;
}

static void amoled_send_1wire(uint8_t cmd, size_t len, uint8_t data[len]) {

    // can pack inst + addr into single 32-bit
    hstx_configure_1wire();
    hstx_put_word(0x02);    // 1 wire write
    hstx_put_word(0x00);
    hstx_put_word(cmd);
    hstx_put_word(0x00);

    // send args
    for(size_t i = 0; i < len; i++) {
        hstx_put_word(data[i]);
    }
}

static void amoled_send_4wire(uint8_t cmd, size_t len, uint8_t data[len]) {

    // can pack inst + addr into single 32-bit
    hstx_configure_1wire();
    hstx_put_word(0x32);    // 4 wire write
    hstx_put_word(0x00);
    hstx_put_word(cmd);
    hstx_put_word(0x00);

    hstx_configure_4wire();
    for(size_t i = 0; i < len; i++) {
        hstx_put_word(data[i]);
    }
}

/*
 * ============================================================================
 * Driver functions
 * ============================================================================
 */

void pw_screen_init() {

    /*
     * Adapted from https://github.com/raspberrypi/pico-examples/blob/master/hstx/spi_lcd/hstx_spi_lcd.c
     * From SPI 1-lane to 4
     */

    /*
     * Set up clocks to 48MHz
     */
    reset_block(RESETS_RESET_HSTX_BITS);
    hw_write_masked(
        &clocks_hw->clk[clk_hstx].ctrl,
        CLOCKS_CLK_HSTX_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB << CLOCKS_CLK_HSTX_CTRL_AUXSRC_LSB,
        CLOCKS_CLK_HSTX_CTRL_AUXSRC_BITS
    );
    unreset_block_wait(RESETS_RESET_HSTX_BITS);

    /*
     * Set up manual CSB
     */
    gpio_init(PIN_HSTX_CSB);
    gpio_set_dir(PIN_HSTX_CSB, GPIO_OUT);
    gpio_put(PIN_HSTX_CSB, 1);

    hstx_configure_1wire();

    gpio_set_function(PIN_HSTX_SCK, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD0, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD1, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD2, 0/*GPIO_FUNC_HSTX*/);
    gpio_set_function(PIN_HSTX_SD3, 0/*GPIO_FUNC_HSTX*/);
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

