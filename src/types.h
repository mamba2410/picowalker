#ifndef PW_TYPES_H
#define PW_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 *  Pokewalker IR packet.
 *  Using this mess of a struct to allow for easier
 *  conversion between the bytes and named fields.
 *  Can access all bytes with packet.bytes[] or can
 *  access individual members as c types with their
 *  field name.
 *
 *  Putting the bytes into a "container" shouldn't
 *  affect their order, just the interpreted value.
 *  So unless you're doing calcs with the *value*
 *  of the checksum/session ID then using their
 *  named fields should be ok.
 *
 *  Still, beware of the host architecture!
 */
typedef struct {
    union {
        struct {
            uint8_t cmd;
            uint8_t extra;
            union {
                uint8_t  checksum_bytes[2];
                uint16_t le_checksum;
            };
            union {
                uint8_t  session_id_bytes[4];
                uint32_t le_session_id;
            };
            uint8_t payload[128];

        };
        uint8_t bytes[0x88];
    };
} pw_packet_t;


typedef struct {
    uint8_t data[0x28];
} unique_identity_data_t;


typedef struct {
    uint8_t data[0x10];
} event_bitmap_t;


typedef struct {
    uint32_t le_unk0;
    uint32_t le_unk1;
    uint16_t le_unk2;
    uint16_t le_unk3;
    uint16_t le_tid;
    uint16_t le_sid;
    unique_identity_data_t identity_data;
    event_bitmap_t event_bitmap;
    uint16_t le_trainer_name[8];
    uint8_t unk4[3];
    uint8_t flags;
    uint8_t protocol_ver;
    uint8_t unk5;
    uint8_t protocol_subver;
    uint8_t unk8;
    uint32_t be_last_sync;
    uint32_t be_step_count;
} walker_info_t;

typedef struct  {
    uint32_t be_total_steps;
    uint32_t be_today_steps;
    uint32_t be_last_sync;
    uint16_t be_total_days;
    uint16_t be_current_watts;
    uint8_t unk[4];
    uint8_t padding[3];
    uint8_t settings;
} health_data_t;

typedef struct {
    uint8_t flags;
    uint8_t commands[0x3f];
} lcd_config_t;

typedef struct {
    uint32_t le_current_steps;
    uint16_t le_current_watts;
    uint8_t padding[2];
    uint32_t le_unk0;
    uint16_t le_unk2;
    uint16_t le_species;
    uint16_t pokemon_name[11];
    uint16_t trainer_name[8];
    uint8_t pokemon_flags_1;
    uint8_t pokemon_flags_2;
} peer_play_data_t;

typedef struct {
    uint16_t le_species;
    uint16_t le_held_item;
    uint16_t le_moves[4];
    uint8_t level;
    uint8_t pokemon_flags_1;
    uint8_t pokemon_flags_2;
    uint8_t padding;
} pokemon_summary_t;

typedef struct {
    uint16_t le_species;
    uint16_t le_held_item;
    uint16_t le_moves[4];
    uint16_t le_ot_tid;
    uint16_t le_ot_sid;
    uint32_t le_pid;
    uint32_t ivs;   // packed to 5-bits each
    uint8_t evs[6];
    uint8_t pokemon_flags_1;
    uint8_t source_game;
    uint8_t ability;
    uint8_t happiness;
    uint8_t level;
    uint8_t padding;
    uint16_t nickname[10];
} pokemon_info_t;

typedef struct {
    uint32_t le_unk0;
    uint16_t le_ot_tid;
    uint16_t le_ot_sid;
    uint16_t le_unk1;
    uint16_t le_location_met;
    uint16_t le_unk2;
    uint16_t ot_name[8];
    uint8_t encounter_type;
    uint8_t ability;
    uint16_t le_pokeball_item;
    uint8_t unk3[10];
} special_pokemon_info_t;

typedef struct {
    uint8_t factory_data[3];
    unique_identity_data_t unique_data;
    uint8_t padding_1;
    lcd_config_t lcd_config;
    uint8_t padding_2; // random 0xbf byte?
    walker_info_t walker_info;
    uint8_t padding_3;
    health_data_t health_data;
    uint8_t padding_4;
    uint8_t copy_marker;
    uint8_t padding[16];
} reliable_data_t;

typedef struct {
    uint8_t stamp_heart: 1;
    uint8_t stamp_space: 1;
    uint8_t stamp_diamond: 1;
    uint8_t stamp_club: 1;
    uint8_t special_map: 1;
    uint8_t event_pokemon: 1;
    uint8_t evet_item: 1;
    uint8_t special_route: 1;
} special_inventory_t;


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



typedef enum {
    EVENT_TYPE_EMPTY_ENTRY,
    EVENT_TYPE_PEER_PLAY_1,
    EVENT_TYPE_PEER_PLAY_2,
    EVENT_TYPE_PEER_PLAY_3,
    EVENT_TYPE_PEER_PLAY_4,
    EVENT_TYPE_PEER_PLAY_5,
    EVENT_TYPE_PEER_PLAY_6,
    EVENT_TYPE_PEER_PLAY_7,
    EVENT_TYPE_PEER_PLAY_8,
    EVENT_TYPE_PEER_PLAY_9,
    EVENT_TYPE_PEER_PLAY_10,
    EVENT_TYPE_ITEM_DOWSED,
    EVENT_TYPE_SPECIAL_ITEM_DOWSED,     //special event item
    EVENT_TYPE_POKEMON_CAUGHT,
    EVENT_TYPE_SPECIAL_POKEMON_CAUGHT,  //special event poke
    EVENT_TYPE_POKEMON_RAN,			    //0x0f
    EVENT_TYPE_POKEMON_LOST,            //lost to radared poke 0x10
    EVENT_TYPE_POKEMON_FOUND_ITEM,      //idle poke finds item 0x11
    EVENT_TYPE_MOOD_HAPPY,
    EVENT_TYPE_MOOD_RUNNING,
    EVENT_TYPE_MOOD_LOOKING_AWAY,
    EVENT_TYPE_MOOD_BORED,
    EVENT_TYPE_MOOD_GO_HOME,
    EVENT_TYPE_POKEMON_JOINED,
    EVENT_TYPE_WALK_ENDED,
    EVENT_TYPE_WALK_STARTED,
    EVENT_TYPE_PLAYED_ALOT,
    EVENT_TYPE_FELL_ASLEEP,
    EVENT_TYPE_ITEM_GIFTED,
} event_log_type_t;

typedef enum {
    ROUTE_IMAGE_FIELD_AND_TREES,
    ROUTE_IMAGE_FOREST_AND_TREES,
    ROUTE_IMAGE_SUBURBS,
    ROUTE_IMAGE_URBAN,
    ROUTE_IMAGE_MOUNTAIN,
    ROUTE_IMAGE_CAVE,
    ROUTE_IMAGE_LAKE,
    ROUTE_IMAGE_BEACH,
} route_image_index_t;


#endif /* PW_TYPES_H */
