#ifndef ICONS_MAP_H
#define ICONS_MAP_H

#ifndef __ASSEMBLER__
#include <stdint.h>

// Base structure (ALL sprites including UI icons)
typedef struct {
    uint16_t eeprom_address;    // Original EEPROM address
    uint32_t bin_offset;        // Offset in merged binary file
    uint32_t size;              // Size in bytes
    uint32_t uncompressed_size; // Size in bytes
    uint16_t width;             // Width in pixels
    uint16_t height;            // Height in pixels
} color_icons_t;

#define ICONS_COUNT 47
#define ICONS_BIN_SIZE 12027

extern const color_icons_t icons_map[ICONS_COUNT];
extern uint8_t color_icons[ICONS_BIN_SIZE];

/********************************************************************************
 * @brief                   Find sprite image data by original EEPROM address using binary search
                            Time complexity: O(log n)
 * @param eeprom_address    address needed for lookup table
 * @return                  Returns pointer to image data in color_icons bin, or NULL if not found
********************************************************************************/
const color_icons_t* find_icon_by_eeprom_address(uint16_t eeprom_address);
//uint8_t* find_icon_by_eeprom_address(uint16_t eeprom_address);

#endif // __ASSEMBLER__

#endif // ICONS_MAP_H
