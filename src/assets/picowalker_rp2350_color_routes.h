#ifndef ROUTES_MAP_H
#define ROUTES_MAP_H

#ifndef __ASSEMBLER__
#include <stdint.h>

// Base structure
typedef struct {
    uint8_t route_index;        // Values (0-7) 0xBF06 Address for Route Index Lookup
    uint32_t bin_offset;        // Offset in merged binary file
    uint32_t size;              // Size in bytes
    uint32_t uncompressed_size; // Size in bytes
    uint16_t width;             // Width in pixels
    uint16_t height;            // Height in pixels
} color_routes_t;

#define ROUTES_COUNT 8
#define ROUTES_BIN_SIZE 2561

extern const color_routes_t routes_map[ROUTES_COUNT];
extern uint8_t color_routes[ROUTES_BIN_SIZE];

/********************************************************************************
 * @brief                   Find sprite image data by route index (0-7)
                            Time complexity: O(1) - direct array access
 * @param eeprom_address    address needed for lookup table
 * @return                  Returns pointer to image data in color_routes bin, or NULL if not found
********************************************************************************/
const color_routes_t* find_route_by_index(uint8_t route_index);
//uint8_t* find_route_by_index(uint8_t route_index);

#endif // __ASSEMBLER__

#endif // ROUTES_MAP_H
