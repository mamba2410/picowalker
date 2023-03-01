#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "screen.h"
#include "eeprom.h"
#include "eeprom_map.h"
#include "globals.h"

/*
 *  Most of the heavy lifting is done by the driver code
 */

// TODO: move to global buffers.h
//static uint8_t *eeprom_buf = 0;

void pw_screen_draw_from_eeprom(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t addr, size_t len) {
    pw_img_t img = {.height=h, .width=w, .data=eeprom_buf, .size=len};
    pw_eeprom_read(addr, eeprom_buf, len);
    pw_screen_draw_img(&img, x, y);
}

void pw_screen_draw_integer(uint32_t n, size_t right_x, size_t y) {

    size_t x = right_x;
    uint32_t m = n;
    do {
        size_t idx = m%10;
        m = m/10;
        x -= 8;
        pw_screen_draw_from_eeprom(
            x, y,
            8, 16,
            PW_EEPROM_ADDR_IMG_DIGITS+PW_EEPROM_SIZE_IMG_CHAR*idx,
            PW_EEPROM_SIZE_IMG_CHAR
        );
    } while(m>0);
}

void pw_screen_draw_time(uint8_t hour, uint8_t minute, uint8_t second, size_t x, size_t y) {
    pw_screen_draw_subtime(hour, x, y, true);
    x += 24;
    pw_screen_draw_subtime(minute, x, y, true);
    x += 24;
    pw_screen_draw_subtime(second, x, y, false);
}

void pw_screen_draw_subtime(uint8_t n, size_t x, size_t y, bool draw_colon) {
    uint8_t idx;

    idx = n/10;
    pw_screen_draw_from_eeprom(
        x, y,
        8, 16,
        PW_EEPROM_ADDR_IMG_DIGITS+PW_EEPROM_SIZE_IMG_CHAR*idx,
        PW_EEPROM_SIZE_IMG_CHAR
    );

    x += 8;
    idx = n%10;
    pw_screen_draw_from_eeprom(
        x, y,
        8, 16,
        PW_EEPROM_ADDR_IMG_DIGITS+PW_EEPROM_SIZE_IMG_CHAR*idx,
        PW_EEPROM_SIZE_IMG_CHAR
    );
    if(draw_colon) {
        x += 8;
        pw_screen_draw_from_eeprom(
            x, y,
            8, 16,
            PW_EEPROM_ADDR_IMG_CHAR_COLON,
            PW_EEPROM_SIZE_IMG_CHAR
        );
    }
}

