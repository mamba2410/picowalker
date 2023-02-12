#include <stdint.h>
#include <stdlib.h>

#include "compression.h"

void pw_compress_data(uint8_t *data, uint8_t *buf, size_t dlen) {

}

/*
 *  data is packet minus 8-byte header
 *  Assume buf can hold decompressed data
 */
int pw_decompress_data(uint8_t *data, uint8_t *buf, size_t dlen) {
    if(data == 0 || buf == 0) return -1;

    size_t c = 0;
    size_t oc = 0;

    uint8_t decomp_type = data[c++];
    if(decomp_type != 0x10)
        return -1;

    // LE size
    uint32_t decomp_size = data[c] | data[c+1] << 8 | data[c+2] << 16;
    if(decomp_size != 128)
        return -1;
    c += 3;

    while(c < dlen) {
        // loop through header
        uint8_t header = data[c++];
        for(uint8_t chunk_idx = (1<<7); chunk_idx > 0; chunk_idx >>= 1) {
            uint8_t cmd = header & chunk_idx;

            if(cmd != 0) {
                // 2-byte backreference
                uint8_t sz = (data[c]>>4) + 3;
                uint16_t backref = (( (data[c]&0x0f) << 8) | (data[c+1] )) + 1;
                c += 2;

                //if(backref > oc)
                //    return -1;

                for(int i = oc-backref; i < sz; i++, oc++)
                    buf[oc] = buf[i];

            } else {
                // 1-byte raw data
                buf[oc++] = data[c++];
            }
        }
    }
    return 0;
}

