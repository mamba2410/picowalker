#ifndef PW_ROUTE_INFO_H
#define PW_ROUTE_INFO_H

#include "trainer_info.h"

typedef struct {
    pokemon_summary_t pokemon_summary;
    uint16_t pokemon_nickname[11];
    uint8_t pokemon_happiness;
    uint8_t route_image_index;
    uint16_t route_name[21];
    pokemon_summary_t route_pokemon[3];
    uint16_t le_route_pokemon_steps[3];
    uint8_t route_pokemon_percent[3];
    uint8_t padding;
    uint16_t le_route_items[10];
    uint16_t le_route_item_steps[10];
    uint8_t route_item_percent[10];
} route_info_t;

typedef struct {
    uint8_t item_info[6];
    uint8_t route_image_index;
    uint8_t padding_1;
    pokemon_summary_t special_pokemon;
    special_pokemon_info_t special_pokemon_extra;
    uint16_t le_special_pokemon_steps;
    uint8_t special_pokemon_percent;
    uint8_t padding_2;
    uint16_t le_special_item;
    uint16_t le_special_item_steps;
    uint8_t special_item_percent;
    uint8_t padding_3[3];
    uint16_t route_name[21];
    uint8_t pokemon_event_number;
    uint8_t item_event_number;
    uint8_t special_pokemon_sprite_data[0x170]; // should be 0x180 bytes, truncated
    uint8_t special_pokemon_name_image[0x140];
    uint8_t special_area_icon[0xc0];
    uint8_t special_area_name_image[0x140];
    uint8_t special_item_name_image[0x180];
} special_route_info_t;

extern route_info_t const *g_route_info;
extern special_route_info_t const *g_special_route_info;

#endif /* PW_ROUTE_INFO_H */
