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

/*
 *  size: 0x5e = 94 bytes
 *  dmitry: struct EventLogItem
 */
typedef struct {
    /* 0x00 */ uint32_t be_time;
    /* 0x04 */ uint32_t le_unk0;
    /* 0x08 */ uint16_t le_unk2;
    /* 0x0a */ uint16_t le_our_species;
    /* 0x0c */ uint16_t le_other_species;
    /* 0x0e */ uint16_t le_extra;
    /* 0x10 */ uint16_t other_trainer_name[8];
    /* 0x20 */ uint16_t our_pokemon_name[11];
    /* 0x36 */ uint16_t other_pokemon_name[11];
    /* 0x4c */ uint8_t  route_image_index;
    /* 0x4d */ uint8_t  pokemon_friendship;
    /* 0x4e */ uint16_t be_our_watts;
    /* 0x50 */ uint16_t be_other_watts;
    /* 0x52 */ uint32_t be_steps;
    /* 0x56 */ uint32_t be_other_steps;
    /* 0x5a */ uint8_t  event_type;
    /* 0x5b */ uint8_t  our_pokemon_flags;
    /* 0x5c */ uint8_t  other_pokemon_flags;
    /* 0x5d */ uint8_t  padding;
} event_log_item_t;

#endif /* PW_ROUTE_INFO_H */
