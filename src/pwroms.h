#ifndef PWROMS_H
#define PWROMS_H


#define FLASHROM_SIZE	48*1024
#define EEPROM_SIZE		64*1024

#ifndef __ASSEMBLER__

extern uint8_t eeprom[EEPROM_SIZE];
extern uint8_t flashrom[FLASHROM_SIZE];

#endif /* __ASSEMBLER__ */

#endif /* PWROMS_H */
