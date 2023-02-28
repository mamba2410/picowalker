#ifndef PW_EEPROM_H
#define PW_EEPROM_H

#include <stdint.h>
#include <stddef.h>

typedef uint16_t eeprom_addr_t;

int pw_eeprom_read(uint16_t addr, uint8_t *buf, size_t len);
int pw_eeprom_write(uint16_t addr, uint8_t *buf, size_t len);
int pw_eeprom_reliable_read(uint16_t addr1, uint16_t addr2, uint8_t *buf, size_t len);
int pw_eeprom_reliable_write(uint16_t addr1, uint16_t addr2, uint8_t *buf, size_t len);
uint8_t pw_eeprom_checksum(uint8_t *buf, size_t len);

#endif /* PW_EEPROM_H */
