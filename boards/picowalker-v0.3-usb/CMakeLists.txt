cmake_minimum_required(VERSION 3.12)

## Include pico cmake file
include(../../pico_sdk_import.cmake)
include(../../pico_extras_import.cmake)

## Set project name
project(picowalker)

## Init pico CMake stuff
pico_sdk_init()

set(MAIN_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../src")
message(${MAIN_SRC_DIR})

## Create executable from dependencies
add_executable(picowalker
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/board_resources.h

    ${MAIN_SRC_DIR}/drivers/accel/bma400_rp2xxx_spi.c
    ${MAIN_SRC_DIR}/drivers/accel/bma400_rp2xxx_spi.h
    ${MAIN_SRC_DIR}/drivers/audio/pwm_rp2xxx.c
    ${MAIN_SRC_DIR}/drivers/battery/bq25628e_rp2xxx_i2c.c
    ${MAIN_SRC_DIR}/drivers/battery/bq25628e_rp2xxx_i2c.h
    ${MAIN_SRC_DIR}/drivers/eeprom/m95512_rp2xxx_spi.c
    ${MAIN_SRC_DIR}/drivers/eeprom/m95512_rp2xxx_spi.h
    ${MAIN_SRC_DIR}/drivers/input/buttons_rp2xxx_gpio.c
    ${MAIN_SRC_DIR}/drivers/input/buttons_rp2xxx_gpio.h
    ${MAIN_SRC_DIR}/drivers/interrupts/rp2xxx_gpio.c
    ${MAIN_SRC_DIR}/drivers/interrupts/rp2xxx_gpio.h
    ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_pio.c
    ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_pio.h
    ${MAIN_SRC_DIR}/drivers/log/onboard_log.c
    ${MAIN_SRC_DIR}/drivers/log/onboard_log.h
    ${MAIN_SRC_DIR}/drivers/screen/sh8601z_rp2xxx_qspi_pio.c
    ${MAIN_SRC_DIR}/drivers/screen/sh8601z_rp2xxx_qspi_pio.h
    ${MAIN_SRC_DIR}/drivers/sleep/dormant_rp2xxx.c
    ${MAIN_SRC_DIR}/drivers/sleep/dormant_rp2xxx.h
    ${MAIN_SRC_DIR}/drivers/time/rp2350_powman.c
    ${MAIN_SRC_DIR}/drivers/usb/msc_disk.c
    ${MAIN_SRC_DIR}/drivers/usb/msc_disk.h
    ${MAIN_SRC_DIR}/drivers/usb/usb_descriptors.c
    ${MAIN_SRC_DIR}/drivers/usb/tusb_config.h ${MAIN_SRC_DIR}/drivers/log/onboard_log.c ${MAIN_SRC_DIR}/assets/picowalker_rp2xxx_inbuilt.S ${MAIN_SRC_DIR}/assets/picowalker_rp2xxx_inbuilt.h
    ${MAIN_SRC_DIR}/assets/pokewalker_rp2xxx_inbuilt.S
    ${MAIN_SRC_DIR}/assets/pokewalker_rp2xxx_inbuilt.c
    ${MAIN_SRC_DIR}/assets/pokewalker_rp2xxx_inbuilt.h

    ${MAIN_SRC_DIR}/picowalker_structures.h
    ${MAIN_SRC_DIR}/picowalker_core.h
)

## PIO stuff for SDK
pico_generate_pio_header(picowalker "${MAIN_SRC_DIR}/drivers/screen/qspi.pio")

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
    hardware_powman
    tinyusb_device
    tinyusb_board
    ${CMAKE_CURRENT_LIST_DIR}/../../lib/libpicowalker-core.a
)

target_include_directories(picowalker PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../src/
    ${CMAKE_CURRENT_LIST_DIR}/../../src/drivers/usb/
)

