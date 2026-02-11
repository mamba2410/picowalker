#ifndef RLE_H
#define RLE_H

#include <stdint.h>
#include <stdlib.h>



/********************************************************************************
 * @brief               Decompress RLE RGB565 sprite data
 * @param compressed    Pointer to compressed RLE data
 * @param output_size   Expected output size in bytes
 * @return              Pointer to decompressed image
********************************************************************************/
uint8_t* rle_decompress_rgb565(const uint8_t* compressed, uint32_t output_size);

#endif // RLE_H