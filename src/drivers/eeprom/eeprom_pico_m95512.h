#ifndef PW_DRIVER_M95512_H
#define PW_DRIVER_M95512_H

#define EEPROM_SCL_PIN      2 // GP 2, phys 4
#define EEPROM_MOSI_PIN     3 // GP 3, phys 5
#define EEPROM_MISO_PIN     4 // GP 4, phys 6
#define EEPROM_CS_PIN       6 // GP 6, phys 9
#define EEPROM_PAGE_SIZE    128

#define CMD_READ        0x03
#define CMD_WRITE       0x02
#define CMD_WREN        0x06    // write enable
#define CMD_WRDI        0x04    // write disable
#define CMD_RDSR        0x05    // read status reg
#define CMD_WRSR        0x01    // write status ref
#define CMD_RDID        0x83    // release from deep power-down

#define STATUS_WIP      (1<<0)  // write in progress
#define STATUS_WEL      (1<<1)  // write enable latch
#define STATUS_WPEN     (1<<7)  // write protect enable
#define STATUS_BP       (3<<2)  // block protect

#endif /* PW_DRIVER_M95512_H */
