#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

#include <hardware/flash.h>
#include <pico/flash.h>
#include <pico/stdlib.h>

#define LOG_LIMIT_BYTES (512*1024) // 
#define FLASH_LOG_START_OFFSET (256*1024) // 256k from start of flash, read at XIP_BASE+offset
#define LOG_PAGE_SIZE FLASH_SECTOR_SIZE // match flash page size
#define N_BUFFERS 2 // Power of 2, shouldn't need more than 2^1

static char log_ram_buffers[N_BUFFERS][LOG_PAGE_SIZE];
static uint8_t current_buffer = 0;
static size_t cursor = 0;
size_t flash_write_offset = FLASH_LOG_START_OFFSET;


/**
 * Taken from <https://github.com/raspberrypi/pico-examples/blob/master/flash/program/flash_program.c#L37>
 */
static void call_flash_range_program(void *param) {
    uint32_t offset = ((uintptr_t*)param)[0];
    const uint8_t *data = (const uint8_t *)((uintptr_t*)param)[1];
    //flash_range_program(offset, data, FLASH_PAGE_SIZE);
    flash_range_program(offset, data, LOG_PAGE_SIZE);
}

static void call_flash_range_erase(void* param) {
    uint32_t offset = (uint32_t)param;
    //flash_range_erase(offset, FLASH_SECTOR_SIZE);
    flash_range_erase(offset, LOG_PAGE_SIZE);
}

/**
 * Commit RAM log to NVM
 * Using pico-examples flash_program from sdk 2.2.0
 */
static void pw_commit_page(uint8_t page) {

    // Erase whatever is already in the page
    int rc = flash_safe_execute(call_flash_range_erase, (void*)flash_write_offset, UINT32_MAX);
    hard_assert(rc == PICO_OK);

    // Reprogram it with what we want
    uintptr_t params[] = {flash_write_offset, (uintptr_t)log_ram_buffers[page]};
    rc = flash_safe_execute(call_flash_range_program, params, UINT32_MAX);
    hard_assert(rc == PICO_OK);
    
    // Increment next offset
    flash_write_offset = (flash_write_offset + LOG_PAGE_SIZE) % LOG_LIMIT_BYTES;
}


void pw_log(char *msg, size_t len) {
    if(cursor + len >= LOG_PAGE_SIZE) {

        // FIll up rest of page with partial message
        memcpy(&log_ram_buffers[current_buffer][cursor], msg, LOG_PAGE_SIZE-cursor);
        len = cursor + len - LOG_PAGE_SIZE;

        // Write full page to NVM
        printf("[Debug] Wrote log page to NVM at 0x%08x\n", flash_write_offset);
        pw_commit_page(current_buffer);
        current_buffer = (current_buffer+1) & N_BUFFERS;
        cursor = 0;
    }

    memcpy(&log_ram_buffers[current_buffer][cursor], msg, len);
    cursor += len;
    printf("[Debug] Cached log in RAM (%d/%d)\n", cursor, LOG_PAGE_SIZE);
}

void pw_log_dump() {
    uart_write_blocking(uart0, (uint8_t*)(XIP_BASE+FLASH_LOG_START_OFFSET), LOG_PAGE_SIZE);
}

