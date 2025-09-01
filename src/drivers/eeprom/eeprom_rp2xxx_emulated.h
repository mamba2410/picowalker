#ifndef PW_DRIVER_EEPROM_RP2XXX_EMULATED_H
#define PW_DRIVER_EEPROM_RP2XXX_EMULATED_H

#include <stdint.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include <stdlib.h>

// Picowalker
#include "picowalker-defs.h"

// Reserve some flash space for EEPROM simulation
// Using last 64KB of 2MB flash (RP2350 has 2MB by default)
#define DRIVER_EEPROM_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - (64 * 1024))  // Last 64KB
#define DRIVER_EEPROM_SIZE (64 * 1024)             // 64KB EEPROM size
#define DRIVER_EEPROM_WRITE_ALIGN   1           // write sizes must be aligned to this many bytes

#ifndef __ASSEMBLER__
// ?
#endif /* __ASSEMBLER__ */

#endif /* PW_DRIVER_EEPROM_RP2XXX_EMULATED_H */