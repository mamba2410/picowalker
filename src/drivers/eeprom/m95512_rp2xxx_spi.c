#include <stdint.h>

#include <string.h>
#include <stdlib.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "board_resources.h"
#include "../../picowalker-defs.h"
#include "m95512_rp2xxx_spi.h"

static void pw_eeprom_cs_enable() {
    // Configure output, drive low
    gpio_set_dir(EEPROM_CSB_PIN, GPIO_OUT);
    gpio_put(EEPROM_CSB_PIN, 0);
}

static void pw_eeprom_cs_disable() {
    // Don't drive high, go to high-z and let pull-up do the work
    gpio_put(EEPROM_CSB_PIN, 1);
    gpio_set_dir(EEPROM_CSB_PIN, GPIO_IN);
}

static void pw_eeprom_wait_for_ready() {
    uint8_t buf[2] = {0, 0};
    do {
        buf[0] = CMD_RDSR;
        pw_eeprom_cs_enable();
        spi_write_blocking(EEPROM_SPI_HW, buf, 1);
        spi_read_blocking(EEPROM_SPI_HW, 0, buf, 1);
        pw_eeprom_cs_disable();
    } while(buf[0] & STATUS_WIP);
}

void pw_eeprom_init() {
    // TODO: needs external pull-up, so don't drive high to avoid contention
    gpio_init(EEPROM_CSB_PIN);
    pw_eeprom_cs_disable();

    board_spi_init();

    uint8_t msg[3];

    msg[0] = CMD_WREN;
    pw_eeprom_cs_enable();
    spi_write_blocking(EEPROM_SPI_HW, msg, 1);
    pw_eeprom_cs_disable();

    msg[0] = CMD_WRSR;
    msg[1] = 0;
    pw_eeprom_cs_enable();
    spi_write_blocking(EEPROM_SPI_HW, msg, 2);
    pw_eeprom_cs_disable();
}

int pw_eeprom_read(eeprom_addr_t addr, uint8_t *buf, size_t len) {
    //printf("EEPROM: reading %04x\n", addr);

    pw_eeprom_wait_for_ready();

    buf[0] = CMD_WRDI;
    pw_eeprom_cs_enable();
    spi_write_blocking(EEPROM_SPI_HW, buf, 1);
    pw_eeprom_cs_disable();


    buf[0] = CMD_READ;
    buf[1] = (uint8_t)(addr>>8);
    buf[2] = (uint8_t)(addr&0xff);

    pw_eeprom_wait_for_ready();


    pw_eeprom_cs_enable();
    spi_write_blocking(EEPROM_SPI_HW, buf, 3);
    int n_read = spi_read_blocking(EEPROM_SPI_HW, 0, buf, len);
    pw_eeprom_cs_disable();

    return n_read;
}

int pw_eeprom_write(eeprom_addr_t addr, uint8_t *buf, size_t len) {
    uint8_t msg[3];
    size_t this_write, bytes_left;
    int n_written = 0;
    eeprom_addr_t addr_end = addr + len;
    //printf("EEPROM: writing %04x\n", addr);


    while(n_written < len) {
        pw_eeprom_wait_for_ready();

        msg[0] = CMD_WREN;
        pw_eeprom_cs_enable();
        spi_write_blocking(EEPROM_SPI_HW, msg, 1);
        pw_eeprom_cs_disable();

        /*
         *  write length is min(bytes_left, bytes_til_end_of_page)
         */
        bytes_left = addr_end - addr;
        this_write = EEPROM_PAGE_SIZE-( addr&(EEPROM_PAGE_SIZE-1) );
        this_write = (this_write < bytes_left)?this_write:bytes_left;

        msg[0] = CMD_WRITE;
        msg[1] = (uint8_t)(addr>>8);
        msg[2] = (uint8_t)(addr&0xff);

        pw_eeprom_cs_enable();
        spi_write_blocking(EEPROM_SPI_HW, msg, 3);
        spi_write_blocking(EEPROM_SPI_HW, buf, this_write);
        pw_eeprom_cs_disable();

        addr += this_write;
        buf += this_write;
        n_written += this_write;
    }

    return n_written;
}

void pw_eeprom_set_area(eeprom_addr_t addr, uint8_t v, size_t len) {
    // get a buffer of value
    uint8_t *buf = malloc(128);
    memset(buf, v, 128);

    // align writes to page size
    size_t write_sz = 128 - (addr&(0x007f));
    int remaining = len;
    while(remaining > 0) {
        pw_eeprom_write(addr, buf, write_sz);
        remaining -= write_sz;
        write_sz = (remaining>128)?128:remaining;
    }

    free(buf);
}

// M95512 has auto-sleep so these do nothing
void pw_eeprom_sleep() { }
void pw_eeprom_wake() { }
