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
    src/drivers/screen_rp2350_gc9a01a.c
    src/drivers/screen_rp2350_gc9a01a.h
    src/drivers/accel_rp2350_qmi8658a.c
    src/drivers/accel_rp2350_qmi8658a.h
    src/drivers/touch_rp2350_cst816s.c
    src/drivers/touch_rp2350_cst816s.h
    src/drivers/ir_pico_pio.c
    src/drivers/ir_pico_pio.h
    #src/drivers/buttons_pico.c
    #src/drivers/buttons_pico.h
    src/drivers/eeprom_pico_ram.S
    src/drivers/eeprom_pico_ram.c
    src/drivers/eeprom_pico_ram.h
    #src/drivers/eeprom_pico_m95512.c        # TODO: Hardware-v0.2
    #src/drivers/eeprom_pico_m95512.h        # TODO: Hardware-v0.2
    src/drivers/timer_pico.c
    src/drivers/extras_pico.S
    src/drivers/extras_pico.h
    #src/drivers/flash_pico_w25q128.c       # TODO: Hardware-v0.2
    #src/drivers/flash_pico_w25q128.h       # TODO: Hardware-v0.2
    src/drivers/flash_pico_internal.c
    src/drivers/flash_pico_internal.S
    src/drivers/flash_pico_internal.h
    #src/drivers/battery_pico_bq25628e.c
    #src/drivers/battery_pico_bq25628e.h
    #src/drivers/gpio_interrupts_pico.c
    #src/drivers/gpio_interrupts_pico.h
    #src/drivers/power_pico.c
    #src/drivers/power_pico.h
    src/drivers/audio_pico_pwm.c            # TODO: Hardware-v0.2
    src/picowalker-defs.h
)

## PIO stuff for SDK
pico_generate_pio_header(picowalker ${CMAKE_CURRENT_LIST_DIR}/src/drivers/qspi.pio)

## Enable usb, uart etc for stdio
#pico_enable_stdio_usb(picowalker 1)
target_compile_definitions(picowalker PRIVATE
    PICO_DEFAULT_UART=0
    PICO_DEFAULT_UART_TX_PIN=16
    PICO_DEFAULT_UART_RX_PIN=17
)
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
    hardware_pwm
    ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
)
