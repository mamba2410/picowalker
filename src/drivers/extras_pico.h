#ifndef PWROMS_H
#define PWROMS_H


#define FLASHROM_SIZE	48*1024
#define EEPROM_SIZE		64*1024

#define SAD_POKEWALKER_SIZE 576

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 *  These are stored in ram (for now)
 */
//extern uint8_t eeprom[EEPROM_SIZE];
//extern uint8_t flashrom[FLASHROM_SIZE];
extern uint8_t sad_pokewalker[SAD_POKEWALKER_SIZE];

#endif /* __ASSEMBLER__ */

#endif /* PWROMS_H */
