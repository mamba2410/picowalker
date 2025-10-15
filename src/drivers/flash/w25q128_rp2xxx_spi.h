#ifndef PW_PICO_FLASH_H
#define PW_PICO_FLASH_H

#include <stdint.h>

#include "../../picowalker-defs.h"

#define FLASH_CSB_PIN   5   // GP5, phys 7

// Partitions
#define ORIGINAL_SPRITES_START  0
#define ORIGINAL_SPRITES_SIZE   0x190
#define COLOUR_SPRITES_START    (1<<12) // Aligned to 4kiB block
#define COLOUR_SPRITES_SIZE     0

enum regs_w25q128 {
    REG_WRITE_EN            = 0x06,
    REG_WRITE_DIS           = 0x04,
    REG_READ_STATUS_1       = 0x05,
    REG_READ_STATUS_2       = 0x35,
    REG_READ_STATUS_3       = 0x15,
    REG_WRITE_STATUS_1      = 0x01,
    REG_WRITE_STATUS_2      = 0x31,
    REG_WRITE_STATUS_3      = 0x11,
    REG_READ_SINGLE         = 0x03,
    REG_FAST_READ_SINGLE    = 0x0b,
    REG_FAST_READ_DUAL      = 0x3b,
    REG_FAST_READ_QUAD      = 0x6b,
    REG_PAGE_PROGRAM        = 0x02,
    REG_SECTOR_ERASE        = 0x20,
    REG_32K_BLOCK_ERASE     = 0x52,
    REG_64K_BLOCK_ERASE     = 0xd8,
    REG_CHIP_ERASE          = 0xc7, // also ox60, this can take from 40-200s
    REG_POWER_DOWN          = 0xb9,
    REG_WAKE_READ_DEVICE_ID = 0xab, // Dual function
    REG_READ_MFGR_DEVICE_ID = 0x90, // Dual function
    REG_READ_MFGR_DEVICE_ID_QUAD = 0x94,    // Dual function
    REG_READ_UNIQUE_ID      = 0x4b,
    REG_ENABLE_RESET        = 0x66,
    REG_RESET               = 0x99,
};

#define REG_STATUS_1_WIP    (1<<0)
#define REG_STATUS_1_WEL    (1<<1)
#define REG_STATUS_2_QEN    (1<<1) // disabled by default on IM/IQ parts

#define MANUFACTURER_ID     0xef // Winbond JEDEC manufacturer ID
#define FLASH_PAGE_SIZE     256

void pw_flash_init();
void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf);


#endif /* PW_PICO_FLASH_H */