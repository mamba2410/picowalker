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
void pw_screen_sleep();
void pw_screen_wake();


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
void pw_eeprom_sleep();
void pw_eeprom_wake();


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
 *  Functions defined by the driver
 */
void pw_flash_sleep();
void pw_flash_wake();

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
void pw_ir_sleep();
void pw_ir_wake();

/*
 *  ==================================================================================
 *  ACCEL
 *  ==================================================================================
 */

/*
 * Types and defines
 */

/*
 *  Functions defined by driver
 */
void pw_accel_init();
uint32_t pw_accel_get_new_steps();
void pw_accel_sleep();
void pw_accel_wake();

/*
 *  ==================================================================================
 *  POWER
 *  ==================================================================================
 */

/*
 * Types and defines
 */
#define PW_BATTERY_STATUS_FLAGS_CHARGING    (1<<0)
#define PW_BATTERY_STATUS_FLAGS_FAULT       (1<<1)
#define PW_BATTERY_LOW_THRESHOLD            (10)

typedef struct pw_battery_status_s {
    uint8_t  percent;
    uint8_t flags;
} pw_battery_status_t;

#define PW_WAKE_REASON_RTC      (1<<0)
#define PW_WAKE_REASON_BATTERY  (1<<1)
#define PW_WAKE_REASON_BUTTON   (1<<2)
#define PW_WAKE_REASON_ACCEL    (1<<3)

typedef uint8_t pw_wake_reason_t;

/*
 *  Functions defined by driver
 */
void pw_power_init();
pw_battery_status_t pw_power_get_battery_status();
void pw_power_enter_sleep();
bool pw_power_should_sleep();
pw_wake_reason_t pw_power_get_wake_reason();


/*
 *  ==================================================================================
 *  TIME
 *  ==================================================================================
 */

/*
 * Types and defines
 */
#define PW_EVENT_EVERY_MINUTE   (1<<0)
#define PW_EVENT_EVERY_HOUR     (1<<1)
#define PW_EVENT_EVERY_DAY      (1<<2)

typedef uint8_t pw_time_events_t;

/*
 *  Functions defined by driver
 */
void pw_time_init_rtc(uint32_t last_sync);
void pw_time_update_rtc(uint32_t last_sync);
pw_time_events_t pw_time_check_for_events();
uint64_t pw_time_now_us();
void pw_time_delay_ms(uint32_t ms);
void pw_time_delay_us(uint32_t us);

#endif /* PW_PICOWALKER_INCLUDE_H */

