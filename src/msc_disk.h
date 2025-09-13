#ifndef MSC_DISK_H
#define MSC_DISK_H


#include <stdint.h>

#define DISK_SECTOR_COUNT 32768
#define DISK_SECTOR_SIZE 512
#define EEPROM_BIN_FIRST_LBA 104
#define EEPROM_BIN_LAST_LBA 231
#define EEPROM_BIN_SIZE_BYTES 65536
#define EEPROM_BIN_ENTRY_OFFSET 32

#define LOG_TXT_FIRST_LBA 232
#define LOG_TXT_LAST_LBA 1255
#define LOG_TXT_SIZE_BYTES 524288
#define LOG_TXT_ENTRY_OFFSET 96

#define BPB_FIRST_LBA 0
#define BPB_LAST_LBA 0

#define FAT1_FIRST_LBA 4
#define FAT1_LAST_LBA 35

#define FAT2_FIRST_LBA 36
#define FAT2_LAST_LBA 67

#define ROOT_DIR_FIRST_LBA 68
#define ROOT_DIR_LAST_LBA 99

#define DISK_BPB_INDEX 0
#define DISK_FAT1_INDEX 1
#define DISK_FAT2_INDEX 2
#define DISK_ROOT_DIR_INDEX 3

extern uint8_t msc_disk[4][512]; // External declaration - actual definition is in msc_disk.c


#endif /* MSC_DISK_H */
