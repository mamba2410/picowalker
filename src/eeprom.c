#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "eeprom.h"
#include "eeprom_map.h"
#include "globals.h"
#include "utils.h"

static const char const NINTENDO_STRING[] = "nintendo";

int pw_eeprom_reliable_read(eeprom_addr_t addr1, eeprom_addr_t addr2, uint8_t *buf, size_t len) {

    uint8_t chk1, chk2;
    uint8_t written_chk1, written_chk2;

    bool area1_ok, area2_ok, area_mismatch;;

    pw_eeprom_read(addr2, buf, len);
    pw_eeprom_read(addr2+len, &written_chk2, 1);
    chk2 = pw_eeprom_checksum(buf, len);
    area2_ok = chk2 == written_chk2;

    pw_eeprom_read(addr1, buf, len);
    pw_eeprom_read(addr1+len, &written_chk1, 1);
    chk1 = pw_eeprom_checksum(buf, len);
    area1_ok = chk1 == written_chk1;

    area_mismatch = chk2 != chk1;

    if(area1_ok) {
        // use area 1

        if(area_mismatch || !area2_ok) {
            // if 1 and 2 are different, or if area 2 is bad
            // assume 1 is newer, overwrite area 2 with area 1
            // TODO: Add this back
            //pw_eeprom_write(addr2, buf, len);
            //pw_eeprom_write(addr2+len, &chk1, 1);
            return 2;   // positive so ok, but telling that area 2 was bad
        }
    } else if(area2_ok) {
        // if area 1 is bad and area 2 is ok
        // use area 2 and overwrite area 1
        pw_eeprom_read(addr2, buf, len);
        // TODO: Add this back
        //pw_eeprom_write(addr1, buf, len);
        //pw_eeprom_write(addr1+len, &chk2, 1);
        return 1;   // positive so ok, but telling that area 1 was bad

    } else {
        // if both are bad, we're buggered. fill both areas with 0xff
        for(size_t i = 0; i < len; i++)
            buf[i] = 0xff;
        chk1 = 0xff;
        chk2 = 0xff;
        // TODO: Add this back
        //pw_eeprom_write(addr1, buf, len);
        //pw_eeprom_write(addr1+len, &chk1, 1);
        //pw_eeprom_write(addr2, buf, len);
        //pw_eeprom_write(addr2+len, &chk2, 1);
        return -1;
    }


    return 0;
}

int pw_eeprom_reliable_write(eeprom_addr_t addr1, eeprom_addr_t addr2, uint8_t *buf, size_t len) {

    uint8_t chk = pw_eeprom_checksum(buf, len);
    pw_eeprom_write(addr1, buf, len);
    pw_eeprom_write(addr1+len, &chk, 1);
    pw_eeprom_write(addr2, buf, len);
    pw_eeprom_write(addr2+len, &chk, 1);

    return 0;
}

uint8_t pw_eeprom_checksum(uint8_t *buf, size_t len) {
    uint8_t chk = 1;
    for(size_t i = 0; i < len; i++)
        chk += buf[i];

    return chk;
}

void pw_eeprom_reset(bool clear_events, bool clear_steps) {
    // these are not tied to structs in the walker
    //health_data_cache.be_today_steps = 0;
    //health_data_cache.be_current_watts = 0;
    //walker_info_cache.status_flags &= ~3;

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(&walker_info_cache),
        sizeof(walker_info_cache)
    );

    walker_info_cache.le_unk0 = 0;
    walker_info_cache.le_unk1 = 0;
    walker_info_cache.le_unk2 = 0;
    walker_info_cache.le_unk3 = 0;
    walker_info_cache.le_tid  = 0;
    walker_info_cache.le_sid  = 0;

    // walker zeroes 0x12 bytes, so also 2 bytes un unk4
    for(size_t i = 0; i < 8; i++) {
        walker_info_cache.le_trainer_name[i] = 0;
    }

    if(clear_events) {
        for(size_t i = 0; i < sizeof(event_bitmap_t); i++) {
            walker_info_cache.event_bitmap.data[i] = 0;
        }
    }

    walker_info_cache.flags = 0;
    walker_info_cache.event_index = 0;
    walker_info_cache.protocol_subver = 0;
    walker_info_cache.unk8 = 0;
    walker_info_cache.be_last_sync = 0;

    pw_eeprom_reliable_read(
        PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_2,
        (uint8_t*)(&walker_info_cache.identity_data),
        sizeof(unique_identity_data_t)
    );

    if(clear_steps) {
        walker_info_cache.be_step_count = 0;
    }


    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_IDENTITY_DATA_1,
        PW_EEPROM_ADDR_IDENTITY_DATA_2,
        (uint8_t*)(&walker_info_cache),
        sizeof(walker_info_cache)
    );

    pw_eeprom_initialise_health_data(clear_steps);

    if(clear_steps) {
        pw_eeprom_set_area(0xce80, 0, 0xd4c);
    } else {
        pw_eeprom_set_area(PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY, 0, 0x64);
        pw_eeprom_set_area(PW_EEPROM_ADDR_EVENT_LOG, 0, PW_EEPROM_SIZE_EVENT_LOG);
        pw_eeprom_set_area(PW_EEPROM_ADDR_HISTORIC_STEP_COUNT, 0, PW_EEPROM_SIZE_HISTORIC_STEP_COUNT);
    }

    if(clear_events) {
        pw_eeprom_set_area(PW_EEPROM_ADDR_RECEIVED_BITFIELD, 0, 0x6c8);
    }

    pw_eeprom_set_area(PW_EEPROM_ADDR_MET_PEER_DATA, 0, PW_EEPROM_SIZE_MET_PEER_DATA);

    pw_eeprom_write(PW_EEPROM_ADDR_NINTENDO, NINTENDO_STRING, PW_EEPROM_SIZE_NINTENDO);
}

void pw_eeprom_initialise_health_data(bool clear_time) {
    if(clear_time) {
        health_data_cache.total_steps = 0;
        health_data_cache.total_days = 0;
        health_data_cache.last_sync = swap_bytes_u32(220924800); // 2nd Jan 2008
        health_data_cache.today_steps = 0;
    }

    health_data_cache.walk_minute_counter = 0;
    health_data_cache.current_watts = 0;
    health_data_cache.steps_this_watt = 0;
    health_data_cache.settings &= 0xa4;
    health_data_cache.settings |= 0x24;
    health_data_cache.event_log_index = 0;

    pw_eeprom_reliable_write(
        PW_EEPROM_ADDR_HEALTH_DATA_1,
        PW_EEPROM_ADDR_HEALTH_DATA_2,
        (uint8_t*)(&health_data_cache),
        sizeof(health_data_t)
    );
}


bool pw_eeprom_check_for_nintendo() {
    uint8_t *buf = eeprom_buf;
    pw_eeprom_read(PW_EEPROM_ADDR_NINTENDO, buf, PW_EEPROM_SIZE_NINTENDO);

    uint8_t i = 0;
    for(i = 0; i < PW_EEPROM_SIZE_NINTENDO; i++) {
        if(buf[i] != NINTENDO_STRING[i]) break;
    }

    return i == PW_EEPROM_SIZE_NINTENDO;
}

