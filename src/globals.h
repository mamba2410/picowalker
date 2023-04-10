#ifndef PW_BUFFERS_H
#define PW_BUFFERS_H

#include <stdint.h>

#include "types.h"

extern uint8_t eeprom_buf[];
extern uint8_t decompression_buf[];
extern pw_packet_t packet_buf;

extern health_data_t health_data_cache;
extern walker_info_t walker_info_cache;
extern walker_info_t peer_info_cache;

#define EEPROM_BUF_SIZE         0x100
#define DECOMPRESSION_BUF_SIZE  0x100
#define PACKET_BUF_SIZE         0x88


#endif /* PW_BUFFERS_H */
