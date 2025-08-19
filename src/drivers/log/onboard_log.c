#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
<<<<<<< HEAD
#include <string.h>
=======
>>>>>>> 69909d8 (logging: add functio to log to flash)

#include <hardware/flash.h>
#include <pico/flash.h>
#include <pico/stdlib.h>

<<<<<<< HEAD
#include "onboard_log.h"

pw_flash_log_t flash_log = {};
=======
#define LOG_LIMIT_BYTES (512*1024) // 
#define FLASH_LOG_START_OFFSET (256*1024) // 256k from start of flash, read at XIP_BASE+offset
#define LOG_PAGE_SIZE FLASH_SECTOR_SIZE // match flash page size
#define N_BUFFERS 2 // Power of 2, shouldn't need more than 2^1

char log_ram_buffers[N_BUFFERS][LOG_PAGE_SIZE];
uint8_t current_buffer = 0;
size_t flash_buffer_cursor = 0;
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
<<<<<<< HEAD
static void pw_commit_page() {

    uintptr_t write_addr = LOG_FLASH_OFFSET + flash_log.flash_write_head;

    // Erase whatever is already in the page
    int rc = flash_safe_execute(call_flash_range_erase, (void*)write_addr, UINT32_MAX);
    hard_assert(rc == PICO_OK);

    // Reprogram it with what we want
    uintptr_t params[] = {write_addr, (uintptr_t)flash_log.buffer};
    rc = flash_safe_execute(call_flash_range_program, params, UINT32_MAX);
    hard_assert(rc == PICO_OK);
    
    // Increment next page write
    flash_log.flash_write_head += LOG_PAGE_SIZE;
    if(flash_log.flash_write_head > LOG_LIMIT_BYTES) {
        printf("[Warn ] Flash log overflowed, wrapping back to zero\n");
        flash_log.flash_write_head = 0;
    }
    
    // RAM page written to flash, so now we reset
    flash_log.ram_write_head = 0;
=======
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
>>>>>>> 69909d8 (logging: add functio to log to flash)
}


void pw_log(char *msg, size_t len) {
<<<<<<< HEAD
    // TODO: Breaks if `len` >= 4096

    if(flash_log.ram_write_head + len >= LOG_PAGE_SIZE) {

        // How much to write before we fill up
        size_t partial_write_len = LOG_PAGE_SIZE - flash_log.ram_write_head;

        // FIll up rest of page with partial message
        memcpy(&flash_log.buffer[flash_log.ram_write_head], msg, partial_write_len);

        // Write full page to NVM
        printf("[Debug] Wrote log page to NVM at 0x%08x\n", flash_log.flash_write_head);
        pw_commit_page();

        // Message is now the remaining portion
        len -= partial_write_len;
        msg += partial_write_len;
    }

    memcpy(&flash_log.buffer[flash_log.ram_write_head], msg, len);
    flash_log.ram_write_head += len;
    printf("[Debug] Cached log in RAM (%d/%d)\n", flash_log.ram_write_head, LOG_PAGE_SIZE);
}

void pw_log_dump() {
    uart_write_blocking(uart0, (uint8_t*)LOG_READ_ADDRESS, LOG_PAGE_SIZE);
=======
    if(cursor + len >= LOG_PAGE_SIZE) {

        // FIll up rest of page with partial message
        memcpy(&log_ram_buffers[current_buffer][flash_buffer_cursor], msg, LOG_PAGE_SIZE-flash_buffer_cursor);
        len = flash_buffer_cursor + len - LOG_PAGE_SIZE;

        // Write full page to NVM
        printf("[Debug] Wrote log page to NVM at 0x%08x\n", flash_write_offset);
        pw_commit_page(current_buffer);
        current_buffer = (current_buffer+1) & N_BUFFERS;
        flash_buffer_cursor = 0;
    }

    memcpy(&log_ram_buffers[current_buffer][flash_buffer_cursor], msg, len);
    flash_buffer_cursor += len;
    printf("[Debug] Cached log in RAM (%d/%d)\n", flash_buffer_cursor, LOG_PAGE_SIZE);
}

void pw_log_dump() {
    uart_write_blocking(uart0, (uint8_t*)(XIP_BASE+FLASH_LOG_START_OFFSET), LOG_PAGE_SIZE);
>>>>>>> 69909d8 (logging: add functio to log to flash)
}

