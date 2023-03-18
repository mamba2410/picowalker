#ifndef PW_UTILS_H
#define PW_UTILS_H

#include <stdint.h>
#include <stddef.h>

#include "states.h"


#define HAVE_POKEMON        (1<<0)  // reg_a
#define CAUGHT_POKEMON_1    (1<<1)
#define CAUGHT_POKEMON_2    (1<<2)
#define CAUGHT_POKEMON_3    (1<<3)
#define FOUND_ITEM_1        (1<<0)  // reg_b
#define FOUND_ITEM_2        (1<<1)
#define FOUND_ITEM_3        (1<<2)
#define HAVE_HEART          (1<<0)  // reg_c (received_bitfield)
#define HAVE_SPADE          (1<<1)
#define HAVE_DIAMOND        (1<<2)
#define HAVE_CLUB           (1<<3)
#define HAVE_EVENT_POKEMON  (1<<5)
#define FOUND_EVENT_ITEM    (1<<6)

inline uint16_t swap_bytes_u16(uint16_t x) {
    uint16_t y = (x>>8) | ((x&0xff)<<8);
    return y;
}

inline uint32_t swap_bytes_u32(uint32_t x) {
    uint32_t y = (x>>24) | ((x&0x00ff0000)>>8) | ((x&0x0000ff00)<<8) | ((x&0xff)<<24);
    return y;
}

void pw_read_inventory(state_vars_t *sv);

int nintendo_to_ascii(uint8_t *str, char* buf, size_t len);

#endif /* PW_UTILS_H */
