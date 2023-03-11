#ifndef PW_TYPES_H
#define PW_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 *  Pokewalker IR packet.
 *  Using this mess of a struct to allow for easier
 *  conversion between the bytes and named fields.
 *  Can access all bytes with packet.bytes[] or can
 *  access individual members as c types with their
 *  field name.
 *
 *  Putting the bytes into a "container" shouldn't
 *  affect their order, just the interpreted value.
 *  So unless you're doing calcs with the *value*
 *  of the checksum/session ID then using their
 *  named fields should be ok.
 *
 *  Still, beware of the host architecture!
 */
typedef struct {
    union {
        struct {
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

        };
        uint8_t bytes[0x88];
    };
} pw_packet_t;

#endif /* PW_TYPES_H */
