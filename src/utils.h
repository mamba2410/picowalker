#ifndef PW_UTILS_H
#define PW_UTILS_H

#include <stdint.h>
#include <stddef.h>

inline uint16_t swap_bytes_u16(uint16_t x) {
    uint16_t y = (x>>8) | ((x&0xff)<<8);
    return y;
}

inline uint32_t swap_bytes_u32(uint32_t x) {
    uint32_t y = (x>>24) | ((x&0x00ff0000)>>8) | ((x&0x0000ff00)<<8) | ((x&0xff)<<24);
    return y;
}

int nintendo_to_ascii(uint8_t *str, char* buf, size_t len);

#endif /* PW_UTILS_H */
