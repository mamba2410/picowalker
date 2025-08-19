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

#include "msc_disk.h"

extern uint32_t flash_write_offset;
extern uint32_t flash_buffer_cursor;
extern char log_ram_buffers[2][4096];
extern uint8_t current_buffer;

// whether host does safe-eject
static bool ejected = false;

// Some MCU doesn't have enough 8KB SRAM to store the whole disk
// We will use Flash as read-only disk with board that has
// CFG_EXAMPLE_MSC_READONLY defined
////#define CFG_EXAMPLE_MSC_READONLY true

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
  (void) lun;

  const char vid[] = "TinyUSB";
  const char pid[] = "Mass Storage";
  const char rev[] = "1.0";

  memcpy(vendor_id  , vid, strlen(vid));
  memcpy(product_id , pid, strlen(pid));
  memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;

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
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
  (void) lun;

  *block_count = DISK_SECTOR_COUNT;
  *block_size  = DISK_SECTOR_SIZE;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void) lun;
  (void) power_condition;

  if ( load_eject )
  {
    if (start)
    {
      // load disk storage
    }else
    {
      // unload disk storage
      ejected = true;
    }
  }

  return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  (void) lun;

  //printf("[Debug] msc reading lba %d offset %d, length %d\n", lba, offset, bufsize);

  // BPB
  if(lba == BPB_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_BPB_INDEX] + offset;
    memcpy(buffer, addr, bufsize);
    return (int32_t)bufsize;
  }

  // FAT1
  if(lba == FAT1_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_FAT1_INDEX] + offset;
    memcpy(buffer, addr, bufsize);
    return (int32_t)bufsize;
  }

  // FAT2
  if(lba == FAT2_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_FAT2_INDEX] + offset;
    memcpy(buffer, addr, bufsize);
    return (int32_t)bufsize;
  }

  // Root dir
  if(lba == ROOT_DIR_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_ROOT_DIR_INDEX] + offset;
    memcpy(buffer, addr, bufsize);
    uint32_t *log_size = (uint32_t*)&buffer[LOG_TXT_ENTRY_OFFSET+32-4];
    *log_size = flash_write_offset - 256*1024 + flash_buffer_cursor;
    return (int32_t)bufsize;
  }

  // EEPROM_BIN
  if(lba >= EEPROM_BIN_FIRST_LBA && lba <= EEPROM_BIN_LAST_LBA) {
      uint16_t eeprom_addr = (lba-EEPROM_BIN_FIRST_LBA)*DISK_SECTOR_SIZE + offset;
      pw_eeprom_read(eeprom_addr, buffer, bufsize);
      return (int32_t)bufsize;
  }

  // LOG_TXT
  if(lba >= LOG_TXT_FIRST_LBA && lba <= LOG_TXT_LAST_LBA) {
      // This does have buffer overflow but oh well
      uint32_t read_start_bytes = (lba-LOG_TXT_FIRST_LBA)*DISK_SECTOR_SIZE + offset;
      uint32_t flash_bytes_written = flash_write_offset-256*1024;

      uint8_t *data = 0;
      if( read_start_bytes < flash_bytes_written) {
        // This part is in flash, get it
        data = XIP_BASE + (256*1024) + read_start_bytes;
        printf("[Log  ] Reading log file offset %u (flash committed %u bytes) ", read_start_bytes, flash_bytes_written);
        printf("from flash\n");
      } else if(read_start_bytes < flash_bytes_written + flash_buffer_cursor) {
        // Not written to flash yet, still in RAM
        data = log_ram_buffers[current_buffer] + read_start_bytes;
        printf("[Log  ] Reading log file offset %u (flash committed %u bytes) ", read_start_bytes, flash_bytes_written);
        printf("from ram\n");
      } else {
        // Not even been generated, send zeros
        memset(buffer, 0, bufsize);
      }

      if(data != 0) {
        memcpy(buffer, data, bufsize);
      }
      //if(lba == 325) *(uint8_t*)(&buffer[bufsize-1]) = 0;
      return (int32_t)bufsize;
  }

  // Check for overflow of offset + bufsize
  if ( offset + bufsize > DISK_SECTOR_SIZE ) {
    return -1;
  }

  // Anything else is zero
  memset(buffer, 0, bufsize);

  return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun)
{
  (void) lun;

#ifdef CFG_EXAMPLE_MSC_READONLY
  return false;
#else
  return true;
#endif
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  (void) lun;

  // BPB
  if(lba == BPB_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_BPB_INDEX] + offset;
    memcpy(addr, buffer, bufsize);
    return (int32_t)bufsize;
  }

  // FAT1
  if(lba == FAT1_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_FAT1_INDEX] + offset;
    memcpy(addr, buffer, bufsize);
    return (int32_t)bufsize;
  }

  // FAT2
  if(lba == FAT2_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_FAT2_INDEX] + offset;
    memcpy(addr, buffer, bufsize);
    return (int32_t)bufsize;
  }

  // Root dir
  if(lba == ROOT_DIR_FIRST_LBA) {
    uint8_t const* addr = msc_disk[DISK_ROOT_DIR_INDEX] + offset;
    memcpy(addr, buffer, bufsize);
    return (int32_t)bufsize;
  }

  // EEPROM_BIN
  if(lba >= EEPROM_BIN_FIRST_LBA && lba <= EEPROM_BIN_LAST_LBA) {
      uint16_t eeprom_addr = (lba-EEPROM_BIN_FIRST_LBA)*DISK_SECTOR_SIZE + offset;
      pw_eeprom_write(eeprom_addr, buffer, bufsize);
      return (int32_t)bufsize;
  }

  // LOG_TXT
  // Discard
  if(lba >= LOG_TXT_FIRST_LBA && lba <= LOG_TXT_LAST_LBA) {
      return (int32_t)bufsize;
  }

  // Check for overflow of offset + bufsize
  if ( offset + bufsize > DISK_SECTOR_SIZE ) {
    return -1;
  }

  printf("[Warn] USB ignoring %d-byte write to LBA %d offset %d\n", bufsize, lba, offset);

  return (int32_t) bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  int32_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, (size_t) resplen);
    }else
    {
      // SCSI output
    }
  }

  return (int32_t) resplen;
}

