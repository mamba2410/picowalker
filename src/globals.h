#ifndef PW_BUFFERS_H
#define PW_BUFFERS_H

#include <stdint.h>

#include "types.h"

extern uint8_t eeprom_buf[];
extern uint8_t decompression_buf[];
extern pw_packet_t packet_buf;

#define EEPROM_BUF_SIZE         0x100
#define DECOMPRESSION_BUF_SIZE  0x100
#define PACKET_BUF_SIZE         0x88


#endif /* PW_BUFFERS_H */
