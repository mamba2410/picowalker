cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

## Include pico cmake file
include(../../pico_sdk_import.cmake)
include(../../pico_extras_import.cmake)

### Set project name
project(picowalker_rp2350lcd128)

### LVGL version options
option(USE_LVGL "Use LVGL" OFF)

### Init pico CMake stuff
pico_sdk_init()

set(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
message(${MAIN_DIR})

set(MAIN_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../src")
message(${MAIN_SRC_DIR})

### Create executable from dependencies
if (USE_LVGL)

  # LVGL Configuration
  set(LV_CONF_PATH ${MAIN_SRC_DIR}/assets/lv_conf.h CACHE STRING "" FORCE)

 # Pico config for Waveshare RP2350-LCD-1.28
  add_compile_definitions(PICO_FLASH_SIZE_BYTES=16777216)  # 16MB = 16 * 1024 * 1024
  add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)
  add_compile_definitions(PICO_FLASH_SPI_CLKDIV=8)
  add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)

  # Override Default Board PINs
  # add_compile_options(-include ${CMAKE_CURRENT_LIST_DIR}/board_resources.h)

  # Compilation subdirectory for Waveshare RP2350-LCD-1.28
  add_subdirectory(../../lib/pico_waveshare_drivers/GC9A01A GC9A01A)
  add_subdirectory(../../lib/pico_waveshare_drivers/CST816S CST816S)
  add_subdirectory(../../lib/pico_waveshare_drivers/QMI8658 QMI8658)
  add_subdirectory(../../lib/lvgl lvgl)

  # Header file directory for Waveshare RP2350-LCD-1.28
  include_directories(${CMAKE_CURRENT_LIST_DIR}) # board_resources.h

  # Make board_resources.h available to all libraries
  target_include_directories(GC9A01A PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  target_include_directories(CST816S PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  target_include_directories(QMI8658 PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  include_directories(../../lib/pico_waveshare_drivers/GC9A01A)
  include_directories(../../lib/pico_waveshare_drivers/CST816S)
  include_directories(../../lib/pico_waveshare_drivers/QMI8658)
  include_directories(../../lib/lvgl)
  
  # Add driver include directories
  include_directories(${MAIN_SRC_DIR}/drivers/accel)
  include_directories(${MAIN_SRC_DIR}/drivers/audio)
  include_directories(${MAIN_SRC_DIR}/drivers/battery)
  include_directories(${MAIN_SRC_DIR}/drivers/eeprom)
  include_directories(${MAIN_SRC_DIR}/drivers/input)
  include_directories(${MAIN_SRC_DIR}/drivers/ir)
  include_directories(${MAIN_SRC_DIR}/drivers/log)
  include_directories(${MAIN_SRC_DIR}/drivers/screen)
  include_directories(${MAIN_SRC_DIR}/drivers/sleep)
  include_directories(${MAIN_SRC_DIR}/drivers/time)
  include_directories(${MAIN_SRC_DIR}/drivers/usb)
  include_directories(${MAIN_SRC_DIR}/assets)

  add_executable(picowalker_rp2350lcd128
      ${CMAKE_CURRENT_LIST_DIR}/main.c
      ${CMAKE_CURRENT_LIST_DIR}/board_resources.h
      ${MAIN_SRC_DIR}/drivers/accel/qmi8658_rp2xxx.c
      ${MAIN_SRC_DIR}/drivers/accel/qmi8658_rp2xxx.h
      ${MAIN_SRC_DIR}/drivers/audio/pwm_rp2xxx.c
      ${MAIN_SRC_DIR}/drivers/battery/adc_rp2xxx.c
      ${MAIN_SRC_DIR}/drivers/battery/adc_rp2xxx.h
      ${MAIN_SRC_DIR}/drivers/eeprom/emulated_rp2xxx.c
      ${MAIN_SRC_DIR}/drivers/eeprom/emulated_rp2xxx.h
      ${MAIN_SRC_DIR}/drivers/input/buttons_rp2xxx_lvgl.c
      ${MAIN_SRC_DIR}/drivers/input/buttons_rp2xxx_lvgl.h

      # ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_pio.c # TODO Switch back if using PIO IR
      # ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_pio.h # TODO Switch back if using PIO IR
      ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_usb_cdc.c
      ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_usb_cdc.h
      ${MAIN_SRC_DIR}/drivers/log/onboard_log.c
      ${MAIN_SRC_DIR}/drivers/log/onboard_log.h
      ${MAIN_SRC_DIR}/drivers/screen/gc9a01a_rp2xxx_lvgl.c
      ${MAIN_SRC_DIR}/drivers/screen/gc9a01a_rp2xxx_lvgl.h
      ${MAIN_SRC_DIR}/drivers/sleep/dormant_rp2xxx_ws.c
      ${MAIN_SRC_DIR}/drivers/sleep/dormant_rp2xxx_ws.h
      ${MAIN_SRC_DIR}/drivers/time/rp2350_powman.c
      ${MAIN_SRC_DIR}/drivers/usb/msc_disk.c
      ${MAIN_SRC_DIR}/drivers/usb/msc_disk.h
      ${MAIN_SRC_DIR}/drivers/usb/usb_descriptors.c
      ${MAIN_SRC_DIR}/drivers/usb/tusb_config.h
      ${MAIN_SRC_DIR}/assets/picowalker_rp2xxx_inbuilt.S 
      ${MAIN_SRC_DIR}/assets/picowalker_rp2xxx_inbuilt.h
      ${MAIN_SRC_DIR}/assets/pokewalker_rp2xxx_inbuilt.S
      ${MAIN_SRC_DIR}/assets/pokewalker_rp2xxx_inbuilt.c
      ${MAIN_SRC_DIR}/assets/pokewalker_rp2xxx_inbuilt.h
      ${MAIN_SRC_DIR}/assets/picowalker_background_240x240_lvgl.c
      ${MAIN_SRC_DIR}/assets/picowalker_background_240x240_lvgl.h

      ${MAIN_SRC_DIR}/picowalker-defs.h
  )

  # Link these libs together
  target_link_libraries(picowalker_rp2350lcd128
      GC9A01A
      CST816S
      QMI8658
      lvgl
      pico_stdlib
      pico_aon_timer
      hardware_sleep
      hardware_spi
      hardware_i2c
      hardware_pio
      hardware_irq
      hardware_timer
      hardware_pwm
      hardware_adc
      tinyusb_device
      tinyusb_board
      ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rp2350/libpicowalker-core.a
  )
else()
  add_executable(picowalker_rp2350lcd128
      ${CMAKE_SOURCE_DIR}/src/main.c
      # ADD Drivers...
  )
  ## Link these libs together
  target_link_libraries(picowalker_rp2350lcd128
    pico_stdlib
    pico_aon_timer
    hardware_sleep
    hardware_spi
    hardware_i2c
    hardware_pio
    hardware_timer
    hardware_pwm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/rp2350/libpicowalker-core.a
  )
endif()

### Add src/ as a header location for tinyusb
target_include_directories(picowalker_rp2350lcd128 PUBLIC
    ${MAIN_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}  # Add board directory for board_resources.h
)

### Enable usb, uart etc for stdio
pico_enable_stdio_usb(picowalker_rp2350lcd128 1)  # Now using USB CDC 0 for debug
target_compile_definitions(picowalker_rp2350lcd128 PRIVATE
    PICO_STDIO_USB_DEFAULT_CRLF=1
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=1000
    LIB_PICO_STDIO_USB=1
)

### Add outputs other than .elf file
pico_add_extra_outputs(picowalker_rp2350lcd128)
