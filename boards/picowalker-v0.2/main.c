#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

//#include "drivers/eeprom/m95512_rp2xxx_spi.h"
#include "picowalker-defs.h"

#include "board_resources.h"

static bool spi_is_inited = false;
static bool i2c_is_inited = false;

void board_spi_init() {
    if(spi_is_inited) return;

    spi_init(COMMON_SPI_HW, COMMON_SPI_SPEED_HZ);
    spi_set_format(COMMON_SPI_HW, 8, 0, 0, SPI_MSB_FIRST);
    gpio_set_function(COMMON_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(COMMON_SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(COMMON_SPI_MISO_PIN, GPIO_FUNC_SPI);

    spi_is_inited = true;
}

void board_i2c_init() {
    if(i2c_is_inited) return;
    i2c_init(COMMON_I2C_HW, COMMON_I2C_SPEED_HZ);
    gpio_set_function(COMMON_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(COMMON_I2C_SCK_PIN, GPIO_FUNC_I2C);
    // hardware pullups
    i2c_is_inited = true;
}

int main() {
    bi_decl(bi_program_description("picowalker"));

    stdio_init_all();
    sleep_ms(1000);
    printf("[Info] ==== Hello, picowalker! ====\n");

    walker_setup();

    extern void (*current_loop)(void);
    while(1) {
        current_loop();
    }

    // unreachable
    while(1);
}

