#ifndef PW_PICOWALKER_INCLUDE_H
#define PW_PICOWALKER_INCLUDE_H

/// @file picowalker-include.h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 *  ==================================================================================
 *  SCREEN
 *  ==================================================================================
 */

/*
 * Types and defines
 */
typedef uint8_t screen_pos_t;       /// 0-95
typedef uint8_t screen_colour_t;    /// 2-bits, pw style

typedef struct {
    screen_pos_t height, width;
    uint8_t *data;
    size_t size; /// bytes
} pw_img_t;

typedef struct {
    screen_pos_t width, height;
    screen_pos_t true_width, true_height;
    screen_pos_t offset_x, offset_y;
} screen_t;

enum {
    SCREEN_WHITE=0,
    SCREEN_LGREY=1,
    SCREEN_DGREY=2,
    SCREEN_BLACK=3,
};

#define SCREEN_WIDTH    96
#define SCREEN_HEIGHT   64

/*
 * Functions defined by the driver
 */
void pw_screen_init();
void pw_screen_draw_img(
    pw_img_t *img,
    screen_pos_t x, screen_pos_t y
);
void pw_screen_clear_area(
    screen_pos_t x, screen_pos_t y,
    screen_pos_t width, screen_pos_t height
);
void pw_screen_draw_horiz_line(
    screen_pos_t x, screen_pos_t y,
    screen_pos_t len,
    screen_colour_t colour
);
void pw_screen_draw_text_box(
    screen_pos_t x1, screen_pos_t y1,
    screen_pos_t x2, screen_pos_t y2,
    screen_colour_t colour
);
void pw_screen_clear();
void pw_screen_fill_area(
    screen_pos_t x, screen_pos_t y,
    screen_pos_t w, screen_pos_t h,
    screen_colour_t colour
);


/*
 *  ==================================================================================
 *  EEPROM
 *  ==================================================================================
 */

/*
 * Types and defines
 */
typedef uint16_t eeprom_addr_t;

/*
 *  Functions defined by the driver
 */
void pw_eeprom_init();
int pw_eeprom_read(eeprom_addr_t addr, uint8_t *buf, size_t len);
int pw_eeprom_write(eeprom_addr_t addr, uint8_t *buf, size_t len);
void pw_eeprom_set_area(eeprom_addr_t addr, uint8_t v, size_t len);


/*
 *  ==================================================================================
 *  FLASH
 *  ==================================================================================
 */

/*
 * Types and defines
 */
typedef enum {
    FLASH_IMG_POKEWALKER,
    FLASH_IMG_FACE_NEUTRAL,
    FLASH_IMG_FACE_HAPPY,
    FLASH_IMG_FACE_SAD,
    FLASH_IMG_UP_ARROW,
    FLASH_IMG_IR_ACTIVE,
    FLASH_IMG_TINY_CHARS,
} pw_flash_img_t;


/*
 *  ==================================================================================
 *  BUTTONS
 *  ==================================================================================
 */

/*
 * Types and defines
 */
enum {
    BUTTON_L = 0x01,
    BUTTON_M = 0x02,
    BUTTON_R = 0x04,
};

#define DEBOUNCE_TIME_US    100000   // 100ms

/*
 *  Functions defined by the driver
 */
void pw_button_init();

/*
 *  ==================================================================================
 *  IR
 *  ==================================================================================
 */

/*
 * Types and defines
 */
#define MAX_PACKET_SIZE (128+8)

#define PW_IR_READ_TIMEOUT_MS   200u
#define PW_IR_READ_TIMEOUT_US   (PW_IR_READ_TIMEOUT_MS*1000)
#define PW_IR_READ_TIMEOUT_DS   (PW_IR_READ_TIMEOUT_MS/100)

/*
 *  Functions defined by the driver
 */
void pw_ir_init();
int pw_ir_read(uint8_t *buf, size_t len);
int pw_ir_write(uint8_t *buf, size_t len);

#endif /* PW_PICOWALKER_INCLUDE_H */
