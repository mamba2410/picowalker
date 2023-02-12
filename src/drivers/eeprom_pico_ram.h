#ifndef PW_DRIVER_EEPROM_PICO_RAM_H
#define PW_DRIVER_EEPROM_PICO_RAM_H


#define DRIVER_EEPROM_SIZE          (64*1024)
#define DRIVER_EEPROM_WRITE_ALIGN   1           // write sizes must be aligned to this many bytes

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>

void pw_eeprom_raw_init();
void pw_eeprom_raw_read(uint16_t addr, uint8_t *buf, size_t len);
void pw_eeprom_raw_write(uint16_t addr, uint8_t *buf, size_t len);


#endif /* __ASSEMBLER__ */


#endif /* PW_DRIVER_EEPROM_PICO_RAM_H */
