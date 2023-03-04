#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "eeprom_25lc512.h"
#include "../eeprom.h"

static spi_inst_t *eeprom_spi;

static void pw_eeprom_cs_enable() {
    gpio_put(EEPROM_CS_PIN, 0);
}

static void pw_eeprom_cs_disable() {
    gpio_put(EEPROM_CS_PIN, 1);
}

static void pw_eeprom_wait_for_ready() {
    uint8_t buf[2];
    do {
        buf[0] = CMD_RDSR;
        pw_eeprom_cs_enable();
        spi_write_blocking(eeprom_spi, buf, 1);
        spi_read_blocking(eeprom_spi, 0, buf, 1);
        pw_eeprom_cs_disable();
    } while(buf[0] & STATUS_WIP);
}

void pw_eeprom_init() {
    eeprom_spi = spi0;

    spi_init(eeprom_spi, 1000*1000);
    // inst, bits, polarity, phase, endian
    spi_set_format(eeprom_spi, 8, 1, 1, SPI_MSB_FIRST);

    gpio_set_function(EEPROM_SCL_PIN, GPIO_FUNC_SPI);
    gpio_set_function(EEPROM_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(EEPROM_MISO_PIN, GPIO_FUNC_SPI);

    gpio_init(EEPROM_CS_PIN);
    gpio_put(EEPROM_CS_PIN, 1);
    gpio_set_dir(EEPROM_CS_PIN, GPIO_OUT);

    uint8_t msg[3];

    msg[0] = CMD_WREN;
    pw_eeprom_cs_enable();
    spi_write_blocking(eeprom_spi, msg, 1);
    pw_eeprom_cs_disable();

    msg[0] = CMD_WRSR;
    msg[1] = 0;
    pw_eeprom_cs_enable();
    spi_write_blocking(eeprom_spi, msg, 2);
    pw_eeprom_cs_disable();
}

int pw_eeprom_read(eeprom_addr_t addr, uint8_t *buf, size_t len) {

    buf[0] = CMD_READ;
    buf[1] = (uint8_t)(addr>>8);
    buf[2] = (uint8_t)(addr&0xff);

    pw_eeprom_wait_for_ready();

    pw_eeprom_cs_enable();
    spi_write_blocking(eeprom_spi, buf, 3);
    int n_read = spi_read_blocking(eeprom_spi, 0, buf, len);
    pw_eeprom_cs_disable();

    return n_read;
}

int pw_eeprom_write(eeprom_addr_t addr, uint8_t *buf, size_t len) {
    uint8_t msg[3];
    size_t this_write, bytes_left;
    int n_written = 0;
    eeprom_addr_t addr_end = addr + len;


    while(n_written < len) {
        pw_eeprom_wait_for_ready();

        msg[0] = CMD_WREN;
        pw_eeprom_cs_enable();
        spi_write_blocking(eeprom_spi, msg, 1);
        pw_eeprom_cs_disable();
        printf("wren\n");

        /*
         *  write length is min(bytes_left, bytes_til_end_of_page)
         */
        bytes_left = addr_end - addr;
        this_write = EEPROM_PAGE_SIZE-( addr&(EEPROM_PAGE_SIZE-1) );
        this_write = (this_write < bytes_left)?this_write:bytes_left;

        msg[0] = CMD_WRITE;
        msg[1] = (uint8_t)(addr>>8);
        msg[2] = (uint8_t)(addr&0xff);

        printf("write %lu\n", this_write);
        pw_eeprom_cs_enable();
        spi_write_blocking(eeprom_spi, msg, 3);
        spi_write_blocking(eeprom_spi, buf, this_write);
        pw_eeprom_cs_disable();

        addr += this_write;
        buf += this_write;
        n_written += this_write;
    }

    return n_written;
}

void pw_eeprom_set_area(eeprom_addr_t addr, uint8_t v, size_t len) {
    uint8_t *buf = malloc(len);
    memset(buf, v, len);
    pw_eeprom_write(addr, buf, len);
    free(buf);
}

