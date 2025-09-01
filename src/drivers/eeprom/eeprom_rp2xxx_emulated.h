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
<<<<<<< HEAD

// New RAM cache management functions
bool pw_eeprom_is_cache_dirty();
const uint8_t* pw_eeprom_get_cache_ptr();
int pw_eeprom_flush_to_flash();
int pw_eeprom_reload_from_flash();
int pw_eeprom_read_flash_direct(eeprom_addr_t addr, uint8_t *buf, size_t len);
size_t pw_eeprom_get_size();

=======
// ?
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
#endif /* __ASSEMBLER__ */

#endif /* PW_DRIVER_EEPROM_RP2XXX_EMULATED_H */