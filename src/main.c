#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "picowalker.h"

int main() {
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
	bi_decl(bi_program_description("picowalker"));

	stdio_init_all();

    printf("Hello, picowalker!\n");

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c / oled_i2d example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#endif

	walker_entry();

}

