#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "eeprom.h"
#include "drivers/eeprom_pico_ram.h"

int pw_eeprom_read(uint16_t addr, uint8_t *buf, size_t len) {
    if(buf == 0) return -1;
    pw_eeprom_raw_read(addr, buf, len);
    return 0;
}

int pw_eeprom_write(uint16_t addr, uint8_t *buf, size_t len) {
    if(buf == 0) return -1;
    pw_eeprom_raw_write(addr, buf, len);
    return 0;
}

int pw_eeprom_reliable_read(uint16_t addr1, uint16_t addr2, uint8_t *buf, size_t len) {

    uint8_t chk1, chk2;
    uint8_t written_chk1, written_chk2;

    bool area1_ok, area2_ok, area_mismatch;;

    pw_eeprom_raw_read(addr2, buf, len);
    pw_eeprom_raw_read(addr2+len, &written_chk2, 1);
    chk2 = pw_eeprom_checksum(buf, len);
    area2_ok = chk2 == written_chk2;

    pw_eeprom_raw_read(addr1, buf, len);
    pw_eeprom_raw_read(addr1+len, &written_chk1, 1);
    chk1 = pw_eeprom_checksum(buf, len);
    area1_ok = chk1 == written_chk1;

    area_mismatch = chk2 != chk1;

    if(area1_ok) {
        // use area 1

        if(area_mismatch || !area2_ok) {
            // if 1 and 2 are different, or if area 2 is bad
            // assume 1 is newer, overwrite area 2 with area 1
            pw_eeprom_raw_write(addr2, buf, len);
            pw_eeprom_raw_write(addr2+len, &chk1, 1);
            return 2;   // positive so ok, but telling that area 2 was bad
        }
    } else if(area2_ok) {
        // if area 1 is bad and area 2 is ok
        // use area 2 and overwrite area 1
        pw_eeprom_raw_read(addr2, buf, len);
        pw_eeprom_raw_write(addr1, buf, len);
        pw_eeprom_raw_write(addr1+len, &chk2, 1);
        return 1;   // positive so ok, but telling that area 1 was bad

    } else {
        // if both are bad, we're buggered. fill both areas with 0xff
        for(size_t i = 0; i < len; i++)
            buf[i] = 0xff;
        chk1 = 0xff;
        chk2 = 0xff;
        pw_eeprom_raw_write(addr1, buf, len);
        pw_eeprom_raw_write(addr1+len, &chk1, 1);
        pw_eeprom_raw_write(addr2, buf, len);
        pw_eeprom_raw_write(addr2+len, &chk2, 1);
        return -1;
    }


    return 0;
}

int pw_eeprom_reliable_write(uint16_t addr1, uint16_t addr2, uint8_t *buf, size_t len) {

    uint8_t chk = pw_eeprom_checksum(buf, len);
    pw_eeprom_raw_write(addr1, buf, len);
    pw_eeprom_raw_write(addr1+len, &chk, 1);
    pw_eeprom_raw_write(addr2, buf, len);
    pw_eeprom_raw_write(addr2+len, &chk, 1);

    return 0;
}

uint8_t pw_eeprom_checksum(uint8_t *buf, size_t len) {
    uint8_t chk = 0;
    for(size_t i = 0; i < len; i++)
        chk += buf[i];

    return chk;
}

