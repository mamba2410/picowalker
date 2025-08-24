cmake_minimum_required(VERSION 3.12)

## Include pico cmake file
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

## Set project name
project(picowalker)

## Init pico CMake stuff
pico_sdk_init()

## Create executable from dependencies
add_executable(picowalker
    src/main.c
    src/drivers/accel/bma400_rp2xxx_spi.c
    src/drivers/accel/bma400_rp2xxx_spi.h
    src/drivers/audio/pwm_rp2xxx.c
    src/drivers/battery/bq25628e_rp2xxx_i2c.c
    src/drivers/battery/bq25628e_rp2xxx_i2c.h
    src/drivers/eeprom/m95512_rp2xxx_spi.c
    src/drivers/eeprom/m95512_rp2xxx_spi.h
    src/drivers/input/buttons_rp2xxx_gpio.c
    src/drivers/input/buttons_rp2xxx_gpio.h
    src/drivers/interrupts/rp2xxx_gpio.c
    src/drivers/interrupts/rp2xxx_gpio.h
    src/drivers/ir/rp2xxx_pio.c
    src/drivers/ir/rp2xxx_pio.h
    src/drivers/screen/sh8601z_rp2xxx_qspi_pio.c
    src/drivers/screen/sh8601z_rp2xxx_qspi_pio.h
    src/drivers/sleep/dormant_rp2xxx.c
    src/drivers/sleep/dormant_rp2xxx.h
    src/drivers/time/rp2350_powman.c
    src/assets/picowalker_rp2xxx_inbuilt.S
    src/assets/picowalker_rp2xxx_inbuilt.h
    src/assets/pokewalker_rp2xxx_inbuilt.S
    src/assets/pokewalker_rp2xxx_inbuilt.c
    src/assets/pokewalker_rp2xxx_inbuilt.h

    src/drivers/log/onboard_log.c
    src/drivers/log/onboard_log.h
    src/drivers/usb/msc_disk.c
    src/drivers/usb/msc_disk.h
    src/drivers/usb/usb_descriptors.c
    src/drivers/usb/tusb_config.h

    src/picowalker-defs.h
)

## PIO stuff for SDK
pico_generate_pio_header(picowalker ${CMAKE_CURRENT_LIST_DIR}/src/drivers/screen/qspi.pio)

## Enable usb, uart etc for stdio
#pico_enable_stdio_usb(picowalker 1)
target_compile_definitions(picowalker PRIVATE
    PICO_DEFAULT_UART=0
    PICO_DEFAULT_UART_TX_PIN=28
    PICO_DEFAULT_UART_RX_PIN=29
)
pico_enable_stdio_uart(picowalker 1)
#pico_enable_stdio_semihosting(picowalker 1)

## Add outputs other than .elf file
pico_add_extra_outputs(picowalker)

## Link these libs together
target_link_libraries(picowalker
	pico_stdlib
    pico_aon_timer
    hardware_sleep
    hardware_spi
    hardware_i2c
    hardware_pio
    hardware_timer
    hardware_pwm
    hardware_dma
    hardware_flash
    ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
)
