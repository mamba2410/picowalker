#ifndef PWROMS_H
#define PWROMS_H

#define FLASHROM_SIZE	48*1024
#define EEPROM_SIZE		64*1024

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>

extern uint8_t *batterY_fancy_text;
extern size_t battery_fancy_text_len;
extern uint8_t *color_fancy_text;
extern size_t color_fancy_text_len;

#endif /* __ASSEMBLER__ */

#endif /* PWROMS_H */
