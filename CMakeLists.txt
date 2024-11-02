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
    src/drivers/screen_pico2_dwo_hstx.c
    src/drivers/screen_pico2_dwo_hstx.h
    src/drivers/accel_pico_bma400.c
    src/drivers/accel_pico_bma400.h
    src/drivers/ir_pico_pio.c
    src/drivers/ir_pico_pio.h
    src/drivers/buttons_pico.c
    src/drivers/buttons_pico.h
    src/drivers/eeprom_pico_m95512.c
    src/drivers/eeprom_pico_m95512.h
    src/drivers/timer_pico.c
    src/drivers/extras_pico.S
    src/drivers/extras_pico.h
    #src/drivers/flash_pico_w25q128.c
    #src/drivers/flash_pico_w25q128.h
    src/drivers/flash_pico_internal.c
    src/drivers/flash_pico_internal.S
    src/drivers/flash_pico_internal.h
    src/drivers/battery_pico_bq25628e.c
    src/drivers/battery_pico_bq25628e.h
    src/picowalker-defs.h
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
    hardware_spi
    hardware_i2c
    ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
)
