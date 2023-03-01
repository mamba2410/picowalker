#include "pico_extra_storage.h"

//.section .data.flashrom, "ax", %progbits
//.type flashrom, %object
//.size flashrom, FLASHROM_SIZE
//
//.global flashrom
//flashrom: .incbin "../rom/pwflash.rom"

/*
// Moved to drivers/eeprom_pico_ram.*
.section .data.eeprom, "ax", %progbits
.type eeprom, %object
.size eeprom, EEPROM_SIZE

.global eeprom
eeprom: .incbin "../rom/pweep.rom"
*/

.global sad_pokewalker
sad_pokewalker: .incbin "../rom/test_image.bin"
