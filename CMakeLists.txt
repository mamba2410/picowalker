cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

## Include pico cmake file
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

## Set project name
project(picowalker)

## Init pico CMake stuff
pico_sdk_init()

# Compilation subdirectory for Waveshare RP2350 Touch LCD 128
add_subdirectory(./lib/RP2350TouchLCD128/Config)
add_subdirectory(./lib/RP2350TouchLCD128/LCD)
add_subdirectory(./lib/RP2350TouchLCD128/Touch)
add_subdirectory(./lib/RP2350TouchLCD128/QMI8658)
add_subdirectory(./lib/lvgl)

# Header file directory for Waveshare RP2350 Touch LCD 128
include_directories(./lib/RP2350TouchLCD128/Config)
include_directories(./lib/RP2350TouchLCD128/LCD)
include_directories(./lib/RP2350TouchLCD128/Touch)
include_directories(./lib/RP2350TouchLCD128/QMI8658)
include_directories(./lib/lvgl)

# Pico config for Waveshare RP2350 Touch LCD 128
add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)
add_compile_definitions(PICO_FLASH_SPI_CLKDIV=8)

## Create executable from dependencies
add_executable(picowalker
    src/main.c
    src/RP2350TouchLCD128.c
    src/RP2350TouchLCD128.h
    src/pokeball.c
    src/pokeball.h
    #src/picowalker-defs.h
)

## PIO stuff for SDK
#pico_generate_pio_header(picowalker ${CMAKE_CURRENT_LIST_DIR}/src/drivers/qspi.pio)

## Enable usb, uart etc for stdio
pico_enable_stdio_usb(picowalker 1)
pico_enable_stdio_uart(picowalker 0)

# target_compile_definitions(picowalker PRIVATE
#     PICO_DEFAULT_UART=0
#     PICO_DEFAULT_UART_TX_PIN=16
#     PICO_DEFAULT_UART_RX_PIN=17
# )
#pico_enable_stdio_semihosting(picowalker 1)


# Add the standard library to the build
#set(LV_CONF_PATH
#  ${CMAKE_CURRENT_SOURCE_DIR}/lib/LVGL/lv_conf.h
#    CACHE STRING "" FORCE)
#add_compile_options(-O2)

## Link these libs together
target_link_libraries(picowalker
    LCD
    Touch
    QMI8658
    lvgl::lvgl
    Config
	  pico_stdlib
    hardware_sleep
    hardware_spi
    hardware_i2c
    hardware_pio
    hardware_timer
    hardware_pwm
    #${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
)

# Add the standard include files to the build
target_include_directories(picowalker   PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

## Add outputs other than .elf file
pico_add_extra_outputs(picowalker)