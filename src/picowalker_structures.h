#ifndef PICOWALKER_STRUCTURES_H
#define PICOWALKER_STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file picowalker_structures.h
 *
 * Common data structures to share between the core and drivers.
 * Drivers treat this as an API and build driver modules to this spec.
 *
 */

/*
 *  ==================================================================================
 *  SCREEN
 *  ==================================================================================
 */

#define PW_SCREEN_WIDTH    96
#define PW_SCREEN_HEIGHT   64

/**
 * Position of something on screen
 * Can be negative, because images can be off screen on the left.
 *
 * Valid range: -PW_SCREEN_WIDTH to PW_SCREEN_WIDTH-1
 */
typedef int8_t pw_screen_pos_t;
typedef uint8_t pw_screen_dim_t;


/**
 * An image data structure for pokewalker images
 * Will change when colour images are implemented
 */
typedef struct pw_img_s {
    uint8_t *data;
    size_t size;
    pw_screen_dim_t height, width;
} pw_img_t;


/**
 * Relates to the original pokewalker image.
 * Give names to the pixel values.
 * Since the original screen is an LCD, a value of 0 is white.
 */
typedef enum pw_screen_color_e {
    PW_SCREEN_WHITE = 0,
    PW_SCREEN_LGREY = 1,
    PW_SCREEN_DGREY = 2,
    PW_SCREEN_BLACK = 3,
} pw_screen_color_t;


/**
 * There are ten brightness pips
 * Value ranges from 0 to 9
 */
#define PW_SCREEN_MAX_BRIGHTNESS 9


/*
 *  ==================================================================================
 *  EEPROM
 *  ==================================================================================
 */

typedef uint16_t pw_eeprom_addr_t;


/*
 *  ==================================================================================
 *  FLASH
 *  ==================================================================================
 */

/**
 * Index of each image stored in flash.
 * Contains only original pokewalker images
 */
typedef enum pw_flash_img_e {
    PW_FLASH_IMG_POKEWALKER,
    PW_FLASH_IMG_FACE_NEUTRAL,
    PW_FLASH_IMG_FACE_HAPPY,
    PW_FLASH_IMG_FACE_SAD,
    PW_FLASH_IMG_UP_ARROW,
    PW_FLASH_IMG_IR_ACTIVE,
    PW_FLASH_IMG_TINY_CHARS,
} pw_flash_img_t;


/*
 *  ==================================================================================
 *  BUTTONS
 *  ==================================================================================
 */

/**
 * Index of buttons.
 * Bits don't overlap so that the consumer can check for multiple pressed.
 */
typedef enum pw_buttons_e {
    PW_BUTTON_L = 0x01,
    PW_BUTTON_M = 0x02,
    PW_BUTTON_R = 0x04,
} pw_buttons_t;


/**
 * Debounce time for buttons.
 * A button pressed less than this time since the last one will be ignored
 */
#define DEBOUNCE_TIME_US    100000   // 100ms


/*
 *  ==================================================================================
 *  IR
 *  ==================================================================================
 */

/**
 * Maximum size of a pokewalker IR packet
 */
#define PACKET_HEADER_SIZE 8
#define PACKET_MAX_PAYLOAD_SIZE 128
#define MAX_PACKET_SIZE (PACKET_HEADER_SIZE+PACKET_MAX_PAYLOAD_SIZE)

/**
 * Timeout value for searching for a byte
 */
#define PW_IR_READ_TIMEOUT_US   200000u

/**
 * Timeout value for the next byte in a packet
 */
#define PW_IR_BYTE_TIMEOUT_US   3742


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

#define PW_POWER_LOW_THRESHOLD            (20)
#define PW_POWER_CRITICAL_THRESHOLD       (10)

#define PW_POWER_STATUS_FLAGS_CHARGING    (1<<0)
#define PW_POWER_STATUS_FLAGS_FAULT       (1<<1)
#define PW_POWER_STATUS_FLAGS_TIMEOUT     (1<<2)
#define PW_POWER_STATUS_FLAGS_MEASUREMENT (1<<3)
#define PW_POWER_STATUS_FLAGS_CHARGE_ENDED    (1<<4)
#define PW_POWER_STATUS_FLAGS_PLUGGED     (1<<5)
#define PW_POWER_STATUS_FLAGS_UNPLUGGED   (1<<6)

/**
 * Status of the power driver
 * `flags` contains various events that couold have happened
 */
typedef struct pw_power_status_s {
    uint8_t flags;
    uint8_t percent;
} pw_power_status_t;


#define PW_WAKE_REASON_RTC      (1<<0)
#define PW_WAKE_REASON_BATTERY  (1<<1)
#define PW_WAKE_REASON_BUTTON   (1<<2)
#define PW_WAKE_REASON_ACCEL    (1<<3)

/**
 * Reason the pokewalker woke up
 * Could be multiple values
 */
typedef uint8_t pw_wake_reason_t;


/*
 *  ==================================================================================
 *  TIME
 *  ==================================================================================
 */

#define RTC_EVENT_EVERY_DAY     (1<<0)
#define RTC_EVENT_EVERY_HOUR    (1<<1)
#define RTC_EVENT_EVERY_MINUTE  (1<<2)
#define RTC_EVENT_EVERY_SECOND  (1<<3)

/**
 * Various events that could happen, related to the RTC
 */
typedef uint8_t pw_rtc_events_t;


/**
 * Days, hours, minutes and seconds
 * Usually as a timestamp, but could also be a delta
 */
typedef struct pw_dhms_s {
    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} pw_dhms_t;


/*
 *  ==================================================================================
 *  AUDIO
 *  ==================================================================================
 */

typedef struct {
    uint8_t info;
    uint8_t period_idx;
} pw_sound_frame_t;

typedef enum {
    VOLUME_NONE=0,
    VOLUME_HALF=1,
    VOLUME_FULL=2
} pw_volume_t;


#endif /* PW_PICOWALKER_INCLUDE_H */

