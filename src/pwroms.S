#include "pwroms.h"

.section .data.flashrom, "ax", %progbits
.type flashrom, %object
.size flashrom, FLASHROM_SIZE

.global flashrom
flashrom: .incbin "../rom/pwflash.rom"

.section .data.eeprom, "ax", %progbits
.type eeprom, %object
.size eeprom, EEPROM_SIZE

.global eeprom
eeprom: .incbin "../rom/pweep.rom"
