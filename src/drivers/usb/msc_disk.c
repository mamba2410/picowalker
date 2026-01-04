/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board_api.h"
#include "tusb.h"

#include "../log/onboard_log.h"
#include "msc_disk.h"
#include "picowalker_structures.h"

extern void pw_eeprom_read(uint16_t addr, uint8_t *buf, size_t len);
extern void pw_eeprom_write(uint16_t addr, const uint8_t *buf, size_t len);

// whether host does safe-eject
static bool ejected = false;

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void)lun;

    const char vid[] = "TinyUSB";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void)lun;

    // RAM disk is ready until ejected
    if (ejected) {
        // Additional Sense 3A-00 is NOT_FOUND
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
        return false;
    }

    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
    (void)lun;

    *block_count = DISK_SECTOR_COUNT;
    *block_size = DISK_SECTOR_SIZE;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void)lun;
    (void)power_condition;

    if (load_eject) {
        if (start) {
            // load disk storage
        } else {
            // unload disk storage
            ejected = true;
        }
    }

    return true;
}

static size_t truncate_chain(uint16_t *fat_slice, uint16_t first_cluster, size_t slice_offset, size_t slice_len,
                             size_t *truncated_length) {
    uint16_t next_cluster = first_cluster;
    uint16_t read_index = next_cluster - slice_offset;
    size_t length = 0;

    if (truncated_length == 0)
        return 0xffff;
    if (first_cluster < slice_offset || first_cluster >= slice_offset + slice_len)
        return 0xffff;

    printf("[Debug] Truncating chain starting at 0x%04x to length of %u\n", first_cluster, *truncated_length);

    for (length = 0;
         length < *truncated_length && next_cluster >= slice_offset && next_cluster - slice_offset < slice_len;
         length++) {
        read_index = next_cluster - slice_offset;
        next_cluster = fat_slice[read_index];
    }

    *truncated_length -= length;

    // Truncate chain
    if (*truncated_length == 0) {
        //printf("[Debug] Truncating by setting FAT entry 0x%04x to 0xffff\n", read_index + slice_offset);
        fat_slice[read_index] = 0xffff;
    } else {
        //printf("[Debug] Didn't reach end of chain, still need %u entries (ended on 0x%04x)\n", *truncated_length,
        //       next_cluster);
    }

    return next_cluster;
}

static uint16_t zero_chain(uint16_t *fat_slice, uint16_t first_cluster, uint16_t slice_offset, uint16_t slice_len) {
    uint16_t next_cluster = first_cluster;

    if (first_cluster == 0xfff8 || first_cluster == 0xffff)
        return first_cluster;

    size_t zero_length = 0;
    while ((next_cluster >= slice_offset) && (next_cluster < slice_offset + slice_len)) {
        uint16_t read_index = next_cluster - slice_offset;
        next_cluster = fat_slice[read_index];
        fat_slice[read_index] = 0x0000;
        zero_length++;
        if (next_cluster == 0xfff8 || next_cluster == 0xffff)
            break;
    }

    if (next_cluster == 0xfff8 || next_cluster == 0xffff) {
        //printf("[Debug] Zeroed %d entries\n", zero_length);
    } else {
        //printf("[Debug] Reached end of buffer, can't zero rest of the chain. Ended on 0x%04x\n", next_cluster);
    }

    return next_cluster;
}

static void edit_fat_table(uint8_t *buffer, size_t bufsize, size_t entries_into_fat) {
    // Truncate cluster chain to actual file length, to satisfy fsck
    size_t log_size = get_apparent_log_size();
    size_t chain_size = (log_size + DISK_CLUSTER_SIZE - 1) / DISK_CLUSTER_SIZE;
    static uint16_t continue_zero = 0xffff;
    static uint16_t continue_trunc = 0xffff;
    static size_t remaining_chain = 0;

    if (entries_into_fat < DISK_SECTOR_SIZE / sizeof(uint16_t)) {
        continue_trunc = LOG_TXT_FIRST_CLUSTER;
        remaining_chain = chain_size;
    }

    if (remaining_chain > 0) {
        uint16_t next_entry = truncate_chain((uint16_t *)buffer, continue_trunc, entries_into_fat,
                                             bufsize / sizeof(uint16_t), &remaining_chain);

        // Chain done, `next_entry` is start of chain to be zeroed
        if (remaining_chain == 0 && next_entry != 0xffff) {
            continue_zero = next_entry;
        } else {
            // Chain not done, `next_entry` is rest of chain to truncate
            continue_trunc = next_entry;
        }
    }

    continue_zero = zero_chain((uint16_t *)buffer, continue_zero, entries_into_fat, bufsize / sizeof(uint16_t));
}

static void handle_fat_request(uint8_t *buffer, size_t bufsize, size_t offset, size_t lba_into_fat) {
    if (lba_into_fat + DISK_FAT_FIRST_INDEX <= DISK_FAT_LAST_INDEX) {
        uint8_t const *addr = msc_disk[DISK_FAT_FIRST_INDEX + lba_into_fat] + offset;

        size_t entries_into_fat = (lba_into_fat * DISK_SECTOR_SIZE + offset) / sizeof(uint16_t);
        memcpy(buffer, addr, bufsize);
        edit_fat_table(buffer, bufsize, entries_into_fat);
    } else {
        memset(buffer, 0, bufsize);
    }
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
    (void)lun;

    // printf("[Debug] msc reading lba %d offset %d, length %d\n", lba, offset, bufsize);

    // BPB
    if (lba == BPB_FIRST_LBA) {
        uint8_t const *addr = msc_disk[DISK_BPB_INDEX] + offset;
        memcpy(buffer, addr, bufsize);
        return (int32_t)bufsize;
    }

    // FAT1
    if (lba >= FAT1_FIRST_LBA && lba <= FAT1_LAST_LBA) {
        size_t lba_into_fat = lba - FAT1_FIRST_LBA;
        handle_fat_request(buffer, bufsize, offset, lba_into_fat);
        return (int32_t)bufsize;
    }

    // FAT2
    if (lba >= FAT2_FIRST_LBA && lba <= FAT2_LAST_LBA) {
        size_t lba_into_fat = lba - FAT2_FIRST_LBA;
        handle_fat_request(buffer, bufsize, offset, lba_into_fat);
        return (int32_t)bufsize;
    }

    // Root dir
    if (lba == ROOT_DIR_FIRST_LBA) {
        uint8_t const *addr = msc_disk[DISK_ROOT_DIR_INDEX] + offset;
        memcpy(buffer, addr, bufsize);
        uint32_t *log_size = (uint32_t *)(buffer + LOG_TXT_ENTRY_OFFSET + 32 - 4);
        //*log_size = flash_log.flash_write_head + flash_log.ram_write_head;
        *log_size = get_apparent_log_size();
        return (int32_t)bufsize;
    }

    // EEPROM_BIN
    if (lba >= EEPROM_BIN_FIRST_LBA && lba <= EEPROM_BIN_LAST_LBA) {
        uint16_t eeprom_addr = (lba - EEPROM_BIN_FIRST_LBA) * DISK_SECTOR_SIZE + offset;
        pw_eeprom_read(eeprom_addr, buffer, bufsize);
        return (int32_t)bufsize;
    }

    // LOG_TXT
    if (lba >= LOG_TXT_FIRST_LBA && lba <= LOG_TXT_LAST_LBA) {

        // This does have buffer overflow but oh well
        uint32_t read_start_bytes = (lba - LOG_TXT_FIRST_LBA) * DISK_SECTOR_SIZE + offset;
        log_read_from_address(read_start_bytes, buffer, bufsize);

        return (int32_t)bufsize;
    }

    // Check for overflow of offset + bufsize
    if (offset + bufsize > DISK_SECTOR_SIZE) {
        return -1;
    }

    // Anything else is zero
    memset(buffer, 0, bufsize);

    return (int32_t)bufsize;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
    (void)lun;

#ifdef CFG_EXAMPLE_MSC_READONLY
    return false;
#else
    return true;
#endif
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
    (void)lun;

    // BPB
    if (lba == BPB_FIRST_LBA) {
        return (int32_t)bufsize;
    }

    // FAT1
    if (lba >= FAT1_FIRST_LBA && lba <= FAT1_LAST_LBA) {
        // Discard
        return (int32_t)bufsize;
    }

    // FAT2
    if (lba >= FAT2_FIRST_LBA && lba <= FAT2_LAST_LBA) {
        // Discard
        return (int32_t)bufsize;
    }

    // Root dir
    if (lba >= ROOT_DIR_FIRST_LBA && lba <= ROOT_DIR_LAST_LBA) {
        // Discard
        return (int32_t)bufsize;
    }

    // EEPROM_BIN
    if (lba >= EEPROM_BIN_FIRST_LBA && lba <= EEPROM_BIN_LAST_LBA) {
        uint16_t eeprom_addr = (lba - EEPROM_BIN_FIRST_LBA) * DISK_SECTOR_SIZE + offset;
        pw_eeprom_write(eeprom_addr, buffer, bufsize);
        return (int32_t)bufsize;
    }

    // LOG_TXT
    if (lba >= LOG_TXT_FIRST_LBA && lba <= LOG_TXT_LAST_LBA) {
        // Discard
        return (int32_t)bufsize;
    }

    // Check for overflow of offset + bufsize
    if (offset + bufsize > DISK_SECTOR_SIZE) {
        return -1;
    }

    printf("[Warn ] USB ignoring %d-byte write to LBA %d offset %d\n", bufsize, lba, offset);

    //return (int32_t)bufsize;
    return -1;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize) {
    // read10 & write10 has their own callback and MUST not be handled here

    void const *response = NULL;
    int32_t resplen = 0;

    // most scsi handled is input
    bool in_xfer = true;

    switch (scsi_cmd[0]) {
    default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

        // negative means error -> tinyusb could stall and/or response with failed status
        resplen = -1;
        break;
    }

    // return resplen must not larger than bufsize
    if (resplen > bufsize)
        resplen = bufsize;

    if (response && (resplen > 0)) {
        if (in_xfer) {
            memcpy(buffer, response, (size_t)resplen);
        } else {
            // SCSI output
        }
    }

    return (int32_t)resplen;
}
