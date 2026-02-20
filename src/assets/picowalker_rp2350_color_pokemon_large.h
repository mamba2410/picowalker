#ifndef POKEMON_LARGE_MAP_H
#define POKEMON_LARGE_MAP_H

#ifndef __ASSEMBLER__
#include <stdint.h>

// Pokemon sprite structure with variant/gender/shiny support
typedef struct {
    // uint16_t species;        // Pokemon species ID (1-493)
    // uint8_t variant_index;   // Form variant (0-31)
    uint32_t composite_key;     // (species << 8) | (is_female << 5) | (variant_index & 0x1F)
    uint32_t bin_offset;        // Offset in merged binary file
    uint32_t size;              // Size in bytes
    uint32_t uncompressed_size; // Size in bytes
    uint16_t width;             // Width in pixels
    uint16_t height;            // Height in pixels
} pokemon_large_entry_t;

#define POKEMON_LARGE_COUNT 651
#define POKEMON_LARGE_BIN_SIZE 2610935

extern const pokemon_large_entry_t pokemon_large_map[POKEMON_LARGE_COUNT];
extern uint8_t color_pokemon_large[POKEMON_LARGE_BIN_SIZE];

/********************************************************************************
 * @brief                   Find Pokemon sprite by species, gender, and variant
 *                          Time complexity: O(log n) binary search
 * @param species           Pokemon species ID (1-493)
 * @param variant_index     Form variant (0-31)
 * @param is_female         Gender flag: 0=male/genderless, 1=female
 * @return                  Returns pointer to sprite data, or NULL if not found
 ********************************************************************************/
const pokemon_large_entry_t* find_pokemon_large(uint16_t species, uint8_t variant_index, uint8_t is_female); 
//uint8_t* find_pokemon_large(uint16_t species, uint8_t variant_index, uint8_t is_female);

#endif // __ASSEMBLER__

#endif // POKEMON_LARGE_MAP_H
