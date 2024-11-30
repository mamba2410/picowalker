#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "stdio.h"

#include "../picowalker-defs.h"
#include "flash_pico.h"

static spi_inst_t *flash_spi;

static const size_t offsets[] = {0x0000, 0x0100, 0x0120, 0x0140, 0x0160, 0x0170, 0x180};
static const size_t sizes[]   = {0x0100, 0x0020, 0x0020, 0x0020, 0x0010, 0x0010, 0x010};

void w25q128_instruction_addr_single(uint8_t instruction, uint32_t addr, size_t len, uint8_t buf[len]) {
    uint8_t instr_buf[4];
    buf[0] = instruction;
    buf[1] = (uint8_t)(addr >> 16); // BE
    buf[2] = (uint8_t)(addr >> 8);
    buf[3] = (uint8_t)(addr >> 0);
    gpio_put(FLASH_CS_PIN, 0);
    spi_write_blocking(flash_spi, instr_buf, 4);
    if(len > 0)
        spi_read_blocking(flash_spi, buf, len);
    gpio_put(FLASH_CS_PIN, 1);

}

void w25q128_instruction_single(uint8_t instruction, size_t len, uint8_t buf[len]) {
    uint8_t instr_buf[4];
    buf[0] = instruction;
    gpio_put(FLASH_CS_PIN, 0);
    spi_write_blocking(flash_spi, instr_buf, 1);
    if(len > 0)
        spi_read_blocking(flash_spi, buf, len);
    gpio_put(FLASH_CS_PIN, 1);

}

/*
 * ============================================================================
 * Functions required by picowalker-core
 * ============================================================================
 */

// TODO: This needs buffer length checking
// TODO: This needs checking that img_index is actually in range
void pw_flash_read(pw_flash_img_t img_index, uint8_t *buf) {
    // Get flash address of image
    size_t flash_address = ORIGINAL_SPRITES_START + offsets[img_index];

    // Perform the read
    w25q128_instruction_addr_single(
        REG_READ_SINGLE,
        flash_address,
        sizes[img_index],
        buf
    );

}

void pw_flash_init() {
    // Uses same SPI bus which is assumed to be set up by eeprom
    // Max read speed 50MHz, can run 00 or 11 mode
    flash_spi = spi0;

    // Set up CS pin
    // May require external pull-up to keep it high during power-up
    gpio_init(FLASH_CS_PIN);
    gpio_set_dir(FLASH_CS_PIN, GPIO_OUT);
    gpio_put(FLASH_CS_PIN, 1);

    // May need 5ms delay between power-on and accepting instructions
    //sleep_ms(5);

    // Check if reads work
    // Wake/read device ID (3 dummy bytes)
    w25q128_instruction_addr_single(REG_WAKE_READ_DEVICE_ID, 1, buf);
    // TODO: delay?

    printf("[Info] External flash chip ID is 0x%02x\n", buf[0]);

    // Reset, put into known state
    // Needs to be awake to reset
    w25q128_instruction_single(REG_RESET_EN, 0, buf);
    w25q128_instruction_single(REG_RESET, 0, buf);
    // TODO: delay 30 us

    // Read some IDs to make sure chip works well now
    w25q128_instruction_addr_single(REG_READ_MFGR_DEVICE_ID, 0, 2, buf);

    if(buf[0] != MANUFACTURER_ID) {
        printf("[Error] External flash manufacturer ID was 0x%02x, expected 0x%02x\n", buf[0], MANUFACTURER_ID);
    }

    printf("[Info] External flash device ID: 0x%02x\n", buf[1]);
    
}


void pw_flash_sleep() {
    // There is a deep sleep mode, but we don't use it for now
    w25q128_instruction_single(REG_POWER_DOWN, 0, buf);
}


void pw_flash_wake() {
    // If we end up using deep sleep, wake up here
    // Read the device ID as well, to make sure we're awake
    w25q128_instruction_addr_single(REG_WAKE_READ_DEVICE_ID, 1, buf);
}

