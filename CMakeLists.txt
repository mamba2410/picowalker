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
	src/picowalker.c
    src/picowalker.h
	src/buttons.c
	src/buttons.h
	src/states.c
	src/states.h
	src/menu.c
	src/menu.h
    src/screen.c
    src/screen.h
    src/globals.c
    src/globals.h
    src/types.h
    src/pico_roms.h
    src/utils.c
    src/utils.h
    src/eeprom.c
    src/eeprom.h
    src/rand.c
    src/rand.h
    src/flash.h
    src/ir/compression.c
    src/ir/compression.h
    src/ir/ir.c
    src/ir/ir.h
    src/ir/actions.c
    src/ir/actions.h
    src/drivers/ssd1327.c
    src/drivers/ssd1327.h
    src/drivers/ir_pico.c
    src/drivers/ir_pico.h
    src/apps/app_comms.c
    src/apps/app_comms.h
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
    src/apps/app_splash.c
    src/apps/app_splash.h
    src/apps/app_trainer_card.c
    src/apps/app_trainer_card.h
    src/apps/app_inventory.c
    src/apps/app_inventory.h
    src/apps/app_dowsing.c
    src/apps/app_dowsing.h
    src/apps/app_first_comms.c
    src/apps/app_first_comms.h
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
)
