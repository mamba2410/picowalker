#ifndef PW_TYPES_H
#define PW_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint8_t cmd;
    uint8_t extra;
    union {
        uint8_t  checksum_bytes[2];
        uint16_t le_checksum;
    };
    union {
        uint8_t  session_id_bytes[4];
        uint32_t le_session_id;
    };
    uint8_t payload[128];
} pw_packet_t;

#endif /* PW_TYPES_H */
