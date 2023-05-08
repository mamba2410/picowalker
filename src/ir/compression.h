#ifndef PW_COMPRESSION_H
#define PW_COMPRESSION_H

#include <stdint.h>
#include <stdlib.h>

/// @file ir/compression.h

void pw_compress_data(uint8_t *data, uint8_t *buf, size_t dlen);
int  pw_decompress_data(uint8_t *data, uint8_t *buf, size_t dlen);

#endif /* PW_COMPRESSION_H */
