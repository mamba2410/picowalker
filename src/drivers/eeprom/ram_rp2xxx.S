#include "eeprom_pico_ram.h"

.section .data.eeprom, "ax", %progbits
.type pw_ram_eeprom, %object
.size pw_ram_eeprom, DRIVER_EEPROM_SIZE

.global pw_ram_eeprom
pw_ram_eeprom: .incbin "../../../rom/pweep.rom"
