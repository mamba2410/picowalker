#ifndef PW_TRAINER_INFO_H
#define PW_TRAINER_INFO_H

#include <stdint.h>

typedef struct {
    uint8_t data[0x28];
} unique_identity_data_t;

typedef struct {
    uint8_t data[0x10];
} event_bitmap_t;

typedef struct {
    uint32_t unk0;
    uint32_t unk1;
    uint16_t unk2;
    uint16_t unk3;
    uint16_t tid;
    uint16_t sid;
    unique_identity_data_t identity_data;
    event_bitmap_t event_bitmap;
    uint16_t trainer_name[8];
    uint8_t unk4[3];
    uint8_t flags;
    uint8_t protocol_ver;
    uint8_t unk5;
    uint8_t protocol_subver;
    uint8_t unk8;
    uint8_t last_sync_time[4];  // u32, big-endian
    uint8_t step_count[4];  // u32, big-endian
} walker_info_t;

typedef struct {
    uint16_t species;
    uint16_t held_item;
    uint16_t moves[4];
    uint8_t level;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t pad1;
} pokemon_summary_t;

typedef struct {
    uint16_t species;
    uint16_t held_item;
    uint16_t moves[4];
    uint16_t ot_tid;
    uint16_t ot_sid;
    uint32_t pid;
    uint32_t ivs;
    uint8_t evs[6];
    uint8_t variant;
    uint8_t source_game;
    uint8_t ability;
    uint8_t happiness;
    uint8_t level;
    uint8_t pad1;
    uint16_t nickname[10];
} pokemon_info_t;



#endif /* PW_TRAINER_INFO_H */
