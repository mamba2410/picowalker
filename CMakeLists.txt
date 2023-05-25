cmake_minimum_required(VERSION 3.12)

## Include pico cmake file
include(pico_sdk_import.cmake)

## Set project name
project(picowalker)

## Init pico CMake stuff
pico_sdk_init()

## Create executable from dependencies
add_executable(picowalker
    src/main.c
    src/drivers/ssd1327.c
    src/drivers/ssd1327.h
    src/drivers/ir_pico.c
    src/drivers/ir_pico.h
    src/drivers/pico_buttons.c
    src/drivers/pico_buttons.h
    #src/drivers/eeprom_pico_ram.S
    #src/drivers/eeprom_pico_ram.c
    #src/drivers/eeprom_pico_ram.h
    src/drivers/eeprom_25lc512.c
    src/drivers/eeprom_25lc512.h
    src/drivers/pico_timer.c
    src/drivers/pico_extra_storage.S
    src/drivers/pico_extra_storage.h
    src/drivers/pico_flash.c
    src/drivers/pico_flash.S
    src/drivers/pico_flash.h
)

## Enable usb, uart etc for stdio
#pico_enable_stdio_usb(picowalker 1)
pico_enable_stdio_uart(picowalker 1)
#pico_enable_stdio_semihosting(picowalker 1)

## Add outputs other than .elf file
pico_add_extra_outputs(picowalker)

## Link these libs together
target_link_libraries(picowalker
	pico_stdlib
    hardware_i2c
    hardware_spi
    ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
)
