#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../eeprom.h"
#include "eeprom_pico_ram.h"


extern uint8_t pw_ram_eeprom[DRIVER_EEPROM_SIZE];

void pw_eeprom_init() {
    // Nothing to do
}

int pw_eeprom_read(eeprom_addr_t addr, uint8_t *buf, size_t len) {
    memcpy(buf, &(pw_ram_eeprom[addr]), len);
    return 0;
}

int pw_eeprom_write(eeprom_addr_t addr, uint8_t *buf, size_t len) {
    memcpy(&(pw_ram_eeprom[addr]), buf, len);
    return 0;
}

void pw_eeprom_set_area(eeprom_addr_t addr, uint8_t v, size_t len) {
    memset(&(pw_ram_eeprom[addr]), v, len);
}
