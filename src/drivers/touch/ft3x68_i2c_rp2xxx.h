#ifndef PW_TOUCH_FT3X68_I2C_RP2XXX_H
#define PW_TOUCH_FT3X68_I2C_RP2XXX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file ft3x68_i2c_rp2xxx.h
 *
 * All of this is taken from the available resources in the 17-page public
 * datasheet from FocalTech and a spreadsheet from DWO when asked what the
 * registers are.
 * There is a fair amount of reverse engineering and guesswork.
 */

#define FT3X68_I2C_ADDR 0x38

#define EXPECTED_FOCALTECH_ID 0x11
#define EXPECTED_G_CIHPER_MID 0x56
#define EXPECTED_G_CIHPER_HIGH 0x64
#define EXPECTED_G_CIPHER_MASK ((EXPECTED_G_CIPHER_HIGH<<16) | (EXPECTED_G_CIPHER_MID<<8))
#define EXPECTED_G_RELEASE_CODE_ID 0x01

typedef enum ft3x68_register_e {
    FT3X68_REG_MODE_SWITCH  = 0x00,
    FT3X68_REG_TD_STATUS    = 0x02,

    FT3X68_REG_P1_XH        = 0x03,
    FT3X68_REG_P1_XL        = 0x04,
    FT3X68_REG_P1_YH        = 0x05,
    FT3X68_REG_P1_YL        = 0x06,
    FT3X68_REG_P1_WEIGHT    = 0x07,
    FT3X68_REG_P1_MISC      = 0x08,

    FT3X68_REG_P2_XH        = 0x09,
    FT3X68_REG_P2_XL        = 0x0a,
    FT3X68_REG_P2_YH        = 0x0b,
    FT3X68_REG_P2_YL        = 0x0c,
    FT3X68_REG_P2_WEIGHT    = 0x0d,
    FT3X68_REG_P2_MISC      = 0x0e,

    FT3X68_REG_ID_G_THGROUP = 0x80,
    FT3X68_REG_ID_G_THDIFF = 0x85,
    FT3X68_REG_ID_G_CTRL = 0x86,
    FT3X68_REG_ID_G_TIMEENTERMONITOR = 0x87,
    FT3X68_REG_ID_G_PERIODACTIVE = 0x88,
    FT3X68_REG_ID_G_PERIODMONITOR = 0x89,
    FT3X68_REG_ID_G_FREQ_HOPPING_EN = 0x8b,
    FT3X68_REG_ID_G_TEST_MODE_FILTER = 0x96,
    FT3X68_REG_ID_G_CIPHER_MID = 0x9f,
    FT3X68_REG_ID_G_CIPHER_LOW = 0xa0,
    FT3X68_REG_ID_G_LIB_VERSION_H = 0xa1,
    FT3X68_REG_ID_G_LIB_VERSION_L = 0xa2,
    FT3X68_REG_ID_G_CIPHER_HIGH = 0xa3,
    FT3X68_REG_ID_G_MODE = 0xa4,
    FT3X68_REG_ID_G_PMODE = 0xa5,
    FT3X68_REG_ID_G_FIRMID = 0xa6,
    FT3X68_REG_ID_G_FOCALTECH_ID = 0xa8,
    FT3X68_REG_ID_G_VIRTUAL_KEY_THRES = 0xa9,
    FT3X68_REG_ID_G_IS_CALLING = 0xad,
    FT3X68_REG_ID_G_FACTORY_MODE = 0xae,
    FT3X68_REG_ID_G_RELEASE_CODE_ID = 0xaf,
    FT3X68_REG_ID_G_FACE_DEC_MODE = 0xb0,
    FT3X68_REG_ID_G_STATE = 0xbc,

    // Debug, marked as "different firmware versions"
    FT3X68_REG_ID_G_SPEC_GESTURE_ENABLE = 0xd0,
    FT3X68_REG_ID_G_SPEC_GESTURE_ID = 0xd3,
} ft3x68_register_t;


/**
 * There are a lot of different ID/version registers.
 * Convenience struct and function to read them all.
 */
typedef struct ft3x68_info_s {
    uint32_t cipher; // 24 bit, G_CIPHER_LOW, G_CIPHER_MID, G_CIPHER_HIGH
    uint16_t lib_version; // LIB_VERSION_L, LIB_VERSION_H
    uint8_t focaltech_id; // G_FOCALTECH_ID
    uint8_t firmware_id; // G_FIRMID
    uint8_t release_code_id; // G_RELEASE_CODE_ID
} ft3x68_info_t;


/**
 * Part ID decode for G_CIPHER_LOW
 */
typedef enum ft3x68_cipher_e {
    FT3x68_CIPHER_FT6456 = 0x00,
    FT3x68_CIPHER_FT3067 = 0x01,
    FT3x68_CIPHER_FT3068 = 0x02,
    FT3x68_CIPHER_FT3168 = 0x03,
    FT3x68_CIPHER_FT3268 = 0x04,
    FT3x68_CIPHER_FT3368 = 0x05,
} ft3x68_cipher_t;


typedef struct ft3x68_point_s {
    uint16_t x; // 12 bits
    uint16_t y; // 12 bits
    uint8_t first_touch_event; // don't know
    uint8_t first_touch_point; // don't know
    uint8_t weight; // unknown
    uint8_t misc; // unknown
} ft3x68_point_t;


typedef enum ft3x68_power_mode_e {
    FT3X68_POWER_MODE_ACTIVE    = 0x00,
    FT3X68_POWER_MODE_MONITOR   = 0x01,
    FT3X68_POWER_MODE_STANDBY   = 0x02,
    FT3X68_POWER_MODE_HIBERNATE = 0x03,
} ft3x68_power_mode_t;


ft3x68_info_t ft3x68_get_part_info();


void ft3x68_peripheral_init();
void ft3x68_reset();
void ft3x68_sleep();

void pw_touch_init();

#endif /* PW_TOUCH_FT3X68_I2C_RP2XXX_H */

