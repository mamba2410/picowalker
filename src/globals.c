#include <stdint.h>

#include "globals.h"
#include "types.h"


uint8_t eeprom_buf[EEPROM_BUF_SIZE];
uint8_t decompression_buf[DECOMPRESSION_BUF_SIZE];
pw_packet_t packet_buf;

