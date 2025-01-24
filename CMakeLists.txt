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
    src/drivers/screen_rp2040_gc9a01a.c
    src/drivers/screen_rp2040_gc9a01a.h
    src/drivers/accel_rp2040_qmi8658a.c
    src/drivers/accel_rp2040_qmi8658a.h
    src/drivers/ir_dummy.c
    src/drivers/ir_dummy.h
    src/drivers/buttons_dummy.c
    src/drivers/buttons_dummy.h
    src/drivers/eeprom_pico_ram.S
    src/drivers/eeprom_pico_ram.c
    src/drivers/eeprom_pico_ram.h
    src/drivers/timer_pico.c
    src/drivers/extras_pico.S
    src/drivers/extras_pico.h
    src/drivers/flash_pico_internal.c
    src/drivers/flash_pico_internal.S
    src/drivers/flash_pico_internal.h
    #src/drivers/battery_pico_bq25628e.c
    #src/drivers/battery_pico_bq25628e.h
    #src/drivers/gpio_interrupts_pico.c
    #src/drivers/gpio_interrupts_pico.h
    src/drivers/power_pico.c
    src/drivers/power_pico.h
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
    hardware_sleep
    hardware_spi
    hardware_i2c
    hardware_pio
    hardware_timer
    ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
)
