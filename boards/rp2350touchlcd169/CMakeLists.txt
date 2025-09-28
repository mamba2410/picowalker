cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

## Include pico cmake file
include(../../pico_sdk_import.cmake)
include(../../pico_extras_import.cmake)

### Set project name
project(picowalker_rp2350touchlcd169)

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

  # Pico config for Waveshare rp2350touchlcd169
  add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)
  add_compile_definitions(PICO_FLASH_SPI_CLKDIV=8)
  add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
  
  # Force include board resources before other headers
  #add_compile_options(-include ${MAIN_SRC_DIR}/src/boards/rp2350touchlcd169/board_resources.h)

  # Compilation subdirectory for Waveshare rp2350touchlcd169

  #add_subdirectory(../../lib/pico_waveshare_drivers/WaveShare WaveShare)
  add_subdirectory(../../lib/pico_waveshare_drivers/ST7789V2 ST7789V2)
  add_subdirectory(../../lib/pico_waveshare_drivers/CST816S CST816S)
  add_subdirectory(../../lib/pico_waveshare_drivers/QMI8658 QMI8658)
  add_subdirectory(../../lib/lvgl lvgl)

  # Header file directory for Waveshare rp2350touchlcd169
  include_directories(${CMAKE_CURRENT_LIST_DIR}) # board_resources.h

  # Make board_resources.h available to all libraries
  target_include_directories(ST7789V2 PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  target_include_directories(CST816S PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  target_include_directories(QMI8658 PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  #include_directories(../../lib/pico_waveshare_drivers/WaveShare)
  include_directories(../../lib/pico_waveshare_drivers/ST7789V2)
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
  # include_directories(${MAIN_SRC_DIR})

  add_executable(picowalker_rp2350touchlcd169
      # ${MAIN_DIR}/src/boards/rp2350touchlcd169/rp2350touchlcd169lvgl.c
      # ${MAIN_DIR}/src/boards/rp2350touchlcd169/rp2350touchlcd169lvgl.h
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

      ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_pio.c # TODO Review
      ${MAIN_SRC_DIR}/drivers/ir/rp2xxx_pio.h # TODO Review
      ${MAIN_SRC_DIR}/drivers/log/onboard_log.c
      ${MAIN_SRC_DIR}/drivers/log/onboard_log.h
      ${MAIN_SRC_DIR}/drivers/screen/st7789v2_rp2xxx_lvgl.c
      ${MAIN_SRC_DIR}/drivers/screen/st7789v2_rp2xxx_lvgl.h
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
  target_link_libraries(picowalker_rp2350touchlcd169
      #WaveShare
      ST7789V2
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
      ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libpicowalker-core.a
  )
else()
  add_executable(picowalker_rp2350touchlcd169
      ${MAIN_DIR}/src/main.c
      ${MAIN_DIR}/src/drivers/screen/screen_rp2xxx_ST7789V2.c
      ${MAIN_DIR}/src/drivers/screen/screen_rp2xxx_ST7789V2.h
      ${MAIN_DIR}/src/drivers/ir/ir_dummy.c
      ${MAIN_DIR}/src/drivers/ir/ir_dummy.h
      ${MAIN_DIR}/src/drivers/buttons/buttons_dummy.c
      ${MAIN_DIR}/src/drivers/buttons/buttons_dummy.h
      ${MAIN_DIR}/src/drivers/eeprom/eeprom_pico_ram.S
      ${MAIN_DIR}/src/drivers/eeprom/eeprom_pico_ram.c
      ${MAIN_DIR}/src/drivers/eeprom/eeprom_pico_ram.h
      ${MAIN_DIR}/src/drivers/timer_pico.c
      ${MAIN_DIR}/src/drivers/extras_pico.S
      ${MAIN_DIR}/src/drivers/extras_pico.h
      ${MAIN_DIR}/src/drivers/flash/flash_pico_internal.c
      ${MAIN_DIR}/src/drivers/flash/flash_pico_internal.S
      ${MAIN_DIR}/src/drivers/flash/flash_pico_internal.h
      #${MAIN_DIR}/src/drivers/battery_pico_bq25628e.c
      #${MAIN_DIR}/src/drivers/battery_pico_bq25628e.h
      #${MAIN_DIR}/src/drivers/gpio_interrupts_pico.c
      #${MAIN_DIR}/src/drivers/gpio_interrupts_pico.h
      ${MAIN_DIR}/src/drivers/power/power_pico.c
      ${MAIN_DIR}/src/drivers/power/power_pico.h
      ${MAIN_DIR}/src/picowalker-defs.h
  )
  ## Link these libs together
  target_link_libraries(picowalker_rp2350touchlcd169
    pico_stdlib
    pico_aon_timer
    hardware_sleep
    hardware_spi
    hardware_i2c
    hardware_pio
    hardware_timer
    hardware_pwm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libpicowalker-core.a
  )
endif()

### Add src/ as a header location for tinyusb
target_include_directories(picowalker_rp2350touchlcd169 PUBLIC
    ${MAIN_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}  # Add board directory for board_resources.h
)

### Enable usb, uart etc for stdio
#pico_enable_stdio_usb(picowalker_rp2350touchlcd169 1)
target_compile_definitions(picowalker_rp2350touchlcd169 PRIVATE
    PICO_DEFAULT_UART=0
    PICO_DEFAULT_UART_TX_PIN=28
    PICO_DEFAULT_UART_RX_PIN=29
)
pico_enable_stdio_uart(picowalker_rp2350touchlcd169 1)

### Add outputs other than .elf file
pico_add_extra_outputs(picowalker_rp2350touchlcd169)
