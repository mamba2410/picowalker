#include <stddef.h>
#include <stdio.h>
#include "picowalker_rp2350_color_rle.h"


uint8_t* rle_decompress_rgb565(const uint8_t* compressed, uint32_t output_size)
{
    static uint8_t output[64 * 96 * 2]; // Max size: 64 * 96 * 2 = 12,288 bytes = 12KB

    uint32_t out_pos = 0;
    uint32_t in_pos = 0;

    while (out_pos < output_size)
    {
        uint8_t count = compressed[in_pos++];

        if (count == 0)  // Literal run
        {
            uint8_t literal_count = compressed[in_pos++];

            // Copy literal pixels (2 bytes each)
            for (uint8_t i = 0; i < literal_count; i++)
            {
                if (out_pos >= output_size) break;
                output[out_pos++] = compressed[in_pos++];  // Low byte
                output[out_pos++] = compressed[in_pos++];  // High byte
            }
        }
        else  // Repeated pixel
        {
            uint8_t pixel_low = compressed[in_pos++];
            uint8_t pixel_high = compressed[in_pos++];

            // Repeat pixel 'count' times
            for (uint8_t i = 0; i < count; i++)
            {
                if (out_pos >= output_size) break;
                output[out_pos++] = pixel_low;
                output[out_pos++] = pixel_high;
            }
        }
    }

    return output;
}