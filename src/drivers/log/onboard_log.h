#ifndef PW_ONBOARD_LOG_H
#define PW_ONBOARD_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <hardware/flash.h>
#include <pico/flash.h>
#include <pico/stdlib.h>

#define LOG_FLASH_OFFSET (256*1024)
#define LOG_READ_ADDRESS ((uint8_t*)(XIP_BASE + LOG_FLASH_OFFSET))
#define LOG_LIMIT_BYTES (512*1024)
#define LOG_PAGE_SIZE (FLASH_SECTOR_SIZE)

/**
 * Values all in bytes
 */
typedef struct pw_flash_log_s {
    size_t ram_write_head;
    size_t flash_write_head;
    size_t flash_read_head;
    uint8_t  buffer[LOG_PAGE_SIZE];
} pw_flash_log_t;

extern pw_flash_log_t flash_log;

size_t get_apparent_log_size();
void log_read_from_address(size_t addr, uint8_t *out, size_t len);


#endif /* PW_ONBOARD_LOG_H */
