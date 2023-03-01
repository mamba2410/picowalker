#ifndef PW_DRIVER_EEPROM_PICO_RAM_H
#define PW_DRIVER_EEPROM_PICO_RAM_H


#define DRIVER_EEPROM_SIZE          (64*1024)
#define DRIVER_EEPROM_WRITE_ALIGN   1           // write sizes must be aligned to this many bytes

#ifndef __ASSEMBLER__

//void pw_eeprom_raw_set_area(uint16_t addr, uint8_t val, size_t len);

#endif /* __ASSEMBLER__ */


#endif /* PW_DRIVER_EEPROM_PICO_RAM_H */
