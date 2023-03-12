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


/*
 *  size: 0x68 = 104 bytes
 *  Dmitry: struct IdentityData
 */
typedef struct {
    /* +0x00 */ uint32_t le_unk0;
    /* +0x04 */ uint32_t le_unk1;
    /* +0x08 */ uint16_t le_unk2;
    /* +0x0a */ uint16_t le_unk3;
    /* +0x0c */ uint16_t le_tid;
    /* +0x0e */ uint16_t le_sid;
    /* +0x10 */ unique_identity_data_t identity_data;
    /* +0x38 */ event_bitmap_t event_bitmap;
    /* +0x48 */ uint16_t le_trainer_name[8];
    /* +0x58 */ uint8_t unk4[3];
    /* +0x5b */ uint8_t flags;
    /* +0x5c */ uint8_t protocol_ver;
    /* +0x5d */ uint8_t unk5;
    /* +0x5e */ uint8_t protocol_subver;
    /* +0x5f */ uint8_t unk8;
    /* +0x60 */ uint32_t be_last_sync;
    /* +0x64 */ uint32_t be_step_count;
} walker_info_t;

/*
 *  size: 0x18 = 24 bytes
 *  dmitry: struct HealthData
 */
typedef struct {    // strut HealthData
    /* +0x00 */ uint32_t be_total_steps;
    /* +0x04 */ uint32_t be_today_steps;
    /* +0x08 */ uint32_t be_last_sync;
    /* +0x0c */ uint16_t be_total_days;
    /* +0x0e */ uint16_t be_current_watts;
    /* +0x10 */ uint8_t unk[4];
    /* +0x14 */ uint8_t padding[3];
    /* +0x17 */ uint8_t settings;
} health_data_t;

typedef struct {
    uint8_t flags;
    uint8_t commands[0x3f];
} lcd_config_t;

/*
 *  size: 0x38 = 56 bytes
 *  dmitry: struct PeerPlayData
 */
typedef struct {    // Dmitry struct PeerPlayData
    /* +0x00 */ uint32_t le_current_steps;
    /* +0x04 */ uint16_t le_current_watts;
    /* +0x06 */ uint8_t padding[2];
    /* +0x08 */ uint32_t le_unk0;
    /* +0x0c */ uint16_t le_unk2;
    /* +0x0e */ uint16_t le_species;
    /* +0x10 */ uint16_t pokemon_name[11];
    /* +0x26 */ uint16_t trainer_name[8];
    /* +0x36 */ uint8_t pokemon_flags_1;
    /* +0x37 */ uint8_t pokemon_flags_2;
} peer_play_data_t;

/*
 *  size: 16 bytes
 *  dmitry: struct PokemonSummary
 */
typedef struct {
    /* +0x00 */ uint16_t le_species;
    /* +0x02 */ uint16_t le_held_item;
    /* +0x04 */ uint16_t le_moves[4];
    /* +0x0c */ uint8_t level;
    /* +0x0d */ uint8_t pokemon_flags_1;    // [0..5] = variant (spinda, arceus, unown, etc.) [6] = female
    /* +0x0e */ uint8_t pokemon_flags_2;    // [0] = has form, [1] = shiny
    /* +0x0f */ uint8_t padding;
} pokemon_summary_t;

/*
 *  size: 0x38 = 56 bytes
 *  dmitry: struct TeamPokeData
 */
typedef struct {
    /* +0x00 */ uint16_t le_species;
    /* +0x02 */ uint16_t le_held_item;
    /* +0x04 */ uint16_t le_moves[4];
    /* +0x0c */ uint16_t le_ot_tid;
    /* +0x0e */ uint16_t le_ot_sid;
    /* +0x10 */ uint32_t le_pid;
    /* +0x14 */ uint32_t ivs;   // packed to 5-bits each
    /* +0x18 */ uint8_t evs[6];
    /* +0x1e */ uint8_t pokemon_flags_1;
    /* +0x1f */ uint8_t source_game;
    /* +0x20 */ uint8_t ability;
    /* +0x21 */ uint8_t happiness;
    /* +0x22 */ uint8_t level;
    /* +0x23 */ uint8_t padding;
    /* +0x24 */ uint16_t nickname[10];
} pokemon_info_t;

/*
 *  size: 0x2c = 44 bytes
 *  dmitry: struct EventPokeExtraData
 */
typedef struct {
    /* +0x00 */ uint32_t le_unk0;
    /* +0x04 */ uint16_t le_ot_tid;
    /* +0x06 */ uint16_t le_ot_sid;
    /* +0x08 */ uint16_t le_unk1;
    /* +0x0a */ uint16_t le_location_met;
    /* +0x0c */ uint16_t le_unk2;
    /* +0x0e */ uint16_t ot_name[8];
    /* +0x1e */ uint8_t encounter_type;
    /* +0x1f */ uint8_t ability;
    /* +0x20 */ uint16_t le_pokeball_item;
    /* +0x22 */ uint8_t unk3[10];
} special_pokemon_info_t;

typedef struct __attribute__((packed)) {
    uint8_t factory_data[2];
    uint8_t factory_data_checksum;
    unique_identity_data_t unique_data;
    uint8_t unique_data_checksum;
    lcd_config_t lcd_config;
    uint8_t lcd_config_checksum; // random 0xbf byte?
    walker_info_t walker_info;
    uint8_t walker_info_checksum;
    health_data_t health_data;
    uint8_t health_data_checksum;
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


/*
 *  size: 0xbe = 190 bytes
 *  dmitry: struct RouteInfo
 */
typedef struct {
    /* +0x00 */ pokemon_summary_t pokemon_summary;
    /* +0x10 */ uint16_t pokemon_nickname[11];
    /* +0x26 */ uint8_t pokemon_happiness;
    /* +0x27 */ uint8_t route_image_index;
    /* +0x28 */ uint16_t route_name[21];
    /* +0x52 */ pokemon_summary_t route_pokemon[3];
    /* +0x82 */ uint16_t le_route_pokemon_steps[3];
    /* +0x88 */ uint8_t route_pokemon_percent[3];
    /* +0x8b */ uint8_t padding;
    /* +0x8c */ uint16_t le_route_items[10];
    /* +0xa0 */ uint16_t le_route_item_steps[10];
    /* +0xb4 */ uint8_t route_item_percent[10];
} route_info_t;


/*
 *  size: 0x6ac = 1708 bytes
 *  dmitry: struct SpecialRoute
 */
typedef struct {
    /* +0x0000 */ uint8_t item_info[6];
    /* +0x0006 */ uint8_t route_image_index;
    /* +0x0007 */ uint8_t padding_1;
    /* +0x0008 */ pokemon_summary_t special_pokemon;
    /* +0x0018 */ special_pokemon_info_t special_pokemon_extra;
    /* +0x0044 */ uint16_t le_special_pokemon_steps;
    /* +0x0046 */ uint8_t special_pokemon_percent;
    /* +0x0047 */ uint8_t padding_2;
    /* +0x0048 */ uint16_t le_special_item;
    /* +0x004a */ uint16_t le_special_item_steps;
    /* +0x004c */ uint8_t special_item_percent;
    /* +0x004d */ uint8_t padding_3[3];
    /* +0x0050 */ uint16_t route_name[21];
    /* +0x007a */ uint8_t pokemon_event_number;
    /* +0x007b */ uint8_t item_event_number;
    /* +0x007c */ uint8_t special_pokemon_sprite_data[0x170]; // should be 0x180 bytes, truncated
    /* +0x01ec */ uint8_t special_pokemon_name_image[0x140];
    /* +0x032c */ uint8_t special_area_icon[0xc0];
    /* +0x03ec */ uint8_t special_area_name_image[0x140];
    /* +0x052c */ uint8_t special_item_name_image[0x180];
} special_route_info_t;

/*
 *  size: 0x88 = 136 bytes
 *  dmitry: struct EventLogItem
 *  Modified from dmitry's version
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
    /* 0x4c */ uint16_t route_name[21];
    /* 0x76 */ uint8_t  route_image_index;
    /* 0x77 */ uint8_t  pokemon_friendship;
    /* 0x78 */ uint16_t be_our_watts;
    /* 0x7a */ uint16_t be_other_watts;
    /* 0x7c */ uint32_t be_steps;
    /* 0x80 */ uint32_t be_other_steps;
    /* 0x84 */ uint8_t  event_type;
    /* 0x85 */ uint8_t  our_pokemon_flags;
    /* 0x86 */ uint8_t  other_pokemon_flags;
    /* 0x87 */ uint8_t  padding;
} event_log_item_t;



typedef enum {
    /* 0x00 */ EVENT_TYPE_EMPTY_ENTRY,
    /* 0x01 */ EVENT_TYPE_PEER_PLAY_1,
    /* 0x02 */ EVENT_TYPE_PEER_PLAY_2,
    /* 0x03 */ EVENT_TYPE_PEER_PLAY_3,
    /* 0x04 */ EVENT_TYPE_PEER_PLAY_4,
    /* 0x05 */ EVENT_TYPE_PEER_PLAY_5,
    /* 0x06 */ EVENT_TYPE_PEER_PLAY_6,
    /* 0x07 */ EVENT_TYPE_PEER_PLAY_7,
    /* 0x08 */ EVENT_TYPE_PEER_PLAY_8,
    /* 0x09 */ EVENT_TYPE_PEER_PLAY_9,
    /* 0x0a */ EVENT_TYPE_PEER_PLAY_10,
    /* 0x0b */ EVENT_TYPE_ITEM_DOWSED,
    /* 0x0c */ EVENT_TYPE_SPECIAL_ITEM_DOWSED,
    /* 0x0d */ EVENT_TYPE_POKEMON_CAUGHT,
    /* 0x0e */ EVENT_TYPE_SPECIAL_POKEMON_CAUGHT,
    /* 0x0f */ EVENT_TYPE_POKEMON_RAN,
    /* 0x10 */ EVENT_TYPE_POKEMON_LOST,
    /* 0x11 */ EVENT_TYPE_POKEMON_FOUND_ITEM,
    /* 0x12 */ EVENT_TYPE_MOOD_HAPPY,
    /* 0x13 */ EVENT_TYPE_MOOD_RUNNING,
    /* 0x14 */ EVENT_TYPE_MOOD_LOOKING_AWAY,
    /* 0x15 */ EVENT_TYPE_MOOD_BORED,
    /* 0x16 */ EVENT_TYPE_MOOD_GO_HOME,
    /* 0x17 */ EVENT_TYPE_POKEMON_JOINED,
    /* 0x18 */ EVENT_TYPE_WALK_ENDED,
    /* 0x19 */ EVENT_TYPE_WALK_STARTED,
    /* 0x1a */ EVENT_TYPE_PLAYED_ALOT,
    /* 0x1b */ EVENT_TYPE_FELL_ASLEEP,
    /* 0x1c */ EVENT_TYPE_ITEM_GIFTED,
} event_log_type_t;

typedef enum {
    /* 0x00 */ ROUTE_IMAGE_FIELD_AND_TREES,
    /* 0x01 */ ROUTE_IMAGE_FOREST_AND_TREES,
    /* 0x02 */ ROUTE_IMAGE_SUBURBS,
    /* 0x03 */ ROUTE_IMAGE_URBAN,
    /* 0x04 */ ROUTE_IMAGE_MOUNTAIN,
    /* 0x05 */ ROUTE_IMAGE_CAVE,
    /* 0x06 */ ROUTE_IMAGE_LAKE,
    /* 0x07 */ ROUTE_IMAGE_BEACH,
} route_image_index_t;


#endif /* PW_TYPES_H */
