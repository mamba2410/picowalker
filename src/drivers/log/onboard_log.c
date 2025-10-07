#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <hardware/flash.h>
#include <pico/flash.h>
#include <pico/stdlib.h>

#include "onboard_log.h"

pw_flash_log_t flash_log = {};


size_t get_flash_used() {
    return (flash_log.flash_write_head - flash_log.flash_read_head + LOG_LIMIT_BYTES) % LOG_LIMIT_BYTES;
}


size_t get_apparent_log_size() {
    return get_flash_used() + flash_log.ram_write_head;
}


void log_read_from_address(size_t addr, uint8_t *out, size_t read_len) {
    // Note: we assume that (addr + read_len) is all valid to access.

    size_t flash_used = get_flash_used();

    // Check if we should read from flash, ram or fill with zeros
    if(addr < flash_used) {
        size_t read_addr = (addr + flash_log.flash_read_head) % LOG_LIMIT_BYTES;
        memcpy(out, XIP_BASE + LOG_FLASH_OFFSET + read_addr, read_len);
    } else if(addr < flash_used + flash_log.ram_write_head) {
        size_t buffer_offset = addr - flash_used;
        memcpy(out, flash_log.buffer + buffer_offset, read_len);
    } else {
        memset(out, 0, read_len);
    }
}

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

    // Ensure that read head is never equal to write head
    if(flash_log.flash_write_head == flash_log.flash_read_head) {
        flash_log.flash_read_head = (flash_log.flash_write_head + LOG_PAGE_SIZE) % LOG_LIMIT_BYTES;
    }
    
    // RAM page written to flash, so now we reset
    flash_log.ram_write_head = 0;
}


void pw_log(char *msg, size_t len) {
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
}

