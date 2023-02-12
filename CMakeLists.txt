cmake_minimum_required(VERSION 3.12)

## Include pico cmake file
include(pico_sdk_import.cmake)

## Set project name
project(picowalker)

## Init pico CMake stuff
pico_sdk_init()

## Create executable from dependencies
add_executable(picowalker
	src/picowalker.c
	src/buttons.c
	src/buttons.h
	src/states.c
	src/states.h
	src/menu.c
	src/menu.h
    src/screen.c
    src/screen.h
    src/pwroms.S
    src/pwroms.h
    src/trainer_info.c
    src/trainer_info.h
    src/route_info.c
    src/route_info.h
    src/utils.c
    src/utils.h
    src/eeprom.c
    src/eeprom.h
    src/rand.c
    src/rand.h
    src/ir/compression.c
    src/ir/compression.h
    src/ir/pw_ir.c
    src/ir/pw_ir.h
    src/ir/pw_ir_actions.c
    src/ir/pw_ir_actions.h
    src/drivers/ssd1327.c
    src/drivers/ssd1327.h
    src/drivers/ir.c
    src/drivers/ir.h
    src/apps/app_comms.c
    src/apps/app_comms.h
    src/drivers/pico_buttons.c
    src/drivers/pico_buttons.h
    src/drivers/eeprom_pico_ram.S
    src/drivers/eeprom_pico_ram.c
    src/drivers/eeprom_pico_ram.h
    src/apps/app_splash.c
    src/apps/app_splash.h
    src/apps/app_trainer_card.c
    src/apps/app_trainer_card.h
    src/apps/app_inventory.c
    src/apps/app_inventory.h
    src/apps/app_dowsing.c
    src/apps/app_dowsing.h
)

## Enable usb, uart etc for stdio
#pico_enable_stdio_usb(picowalker 1)
pico_enable_stdio_uart(picowalker 1)

## Add outputs other than .elf file
pico_add_extra_outputs(picowalker)

## Link these libs together
target_link_libraries(picowalker
	pico_stdlib
    hardware_i2c
)
