#include <stdint.h>
#include <stddef.h>

#include "eeprom_pico_ram.h"


extern uint8_t pw_ram_eeprom[DRIVER_EEPROM_SIZE];

void pw_eeprom_raw_init() {
    // Nothing to do
}

void pw_eeprom_raw_read(uint16_t addr, uint8_t *buf, size_t len) {

}

void pw_eeprom_raw_write(uint16_t addr, uint8_t *buf, size_t len) {

}
