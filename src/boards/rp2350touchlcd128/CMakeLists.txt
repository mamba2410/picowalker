cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set source directory to project root
set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

### Include pico cmake file
include(${CMAKE_SOURCE_DIR}/pico_sdk_import.cmake)
include(${CMAKE_SOURCE_DIR}/pico_extras_import.cmake)

### Set project name
<<<<<<< HEAD
project(picowalker_rp2350touchlcd128)
=======
project(picowalker)
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)

### LVGL version options
option(USE_LVGL "Use LVGL" OFF)

### Init pico CMake stuff
pico_sdk_init()

### Create executable from dependencies
if (USE_LVGL)

  # LVGL Configuration
  set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/src/globals/lv_conf.h CACHE STRING "" FORCE)

  # Pico config for Waveshare RP2350TouchLCD128
  add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)
  add_compile_definitions(PICO_FLASH_SPI_CLKDIV=8)
  add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
  
  # Force include board resources before other headers
  add_compile_options(-include ${CMAKE_SOURCE_DIR}/src/boards/rp2350touchlcd128/board_resources.h)

  # Compilation subdirectory for Waveshare RP2350TouchLCD128
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/WaveShare WaveShare)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/GC9A01A GC9A01A)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/CST816S CST816S)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/QMI8658 QMI8658)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/lvgl lvgl)

  # Header file directory for Waveshare RP2350TouchLCD128
  include_directories(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/WaveShare)
  include_directories(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/GC9A01A)
  include_directories(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/CST816S)
  include_directories(${CMAKE_SOURCE_DIR}/lib/pico_waveshare_drivers/QMI8658)
  include_directories(${CMAKE_SOURCE_DIR}/lib/lvgl)
  
  # Add driver include directories
  include_directories(${CMAKE_SOURCE_DIR}/src/boards/rp2350touchlcd128)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/battery)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/screen)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/accel)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/flash)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/ir)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/power)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/buttons)
  include_directories(${CMAKE_SOURCE_DIR}/src/drivers/eeprom)
  include_directories(${CMAKE_SOURCE_DIR}/src/globals)
  include_directories(${CMAKE_SOURCE_DIR}/src)

<<<<<<< HEAD
  add_executable(picowalker_rp2350touchlcd128
=======
  add_executable(picowalker
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
      # ${CMAKE_SOURCE_DIR}/src/boards/rp2350touchlcd128/rp2350touchlcd128lvgl.c
      # ${CMAKE_SOURCE_DIR}/src/boards/rp2350touchlcd128/rp2350touchlcd128lvgl.h
      ${CMAKE_SOURCE_DIR}/src/main_lvgl.c
      ${CMAKE_SOURCE_DIR}/src/boards/rp2350touchlcd128/board_resources.h
      ${CMAKE_SOURCE_DIR}/src/drivers/accel/accel_rp2xxx_qmi8658.c
      ${CMAKE_SOURCE_DIR}/src/drivers/accel/accel_rp2xxx_qmi8658.h
      ${CMAKE_SOURCE_DIR}/src/drivers/audio/audio_pico_pwm.c
      ${CMAKE_SOURCE_DIR}/src/drivers/battery/battery_rp2xxx_simple.c
      ${CMAKE_SOURCE_DIR}/src/drivers/battery/battery_rp2xxx_simple.h
      ${CMAKE_SOURCE_DIR}/src/drivers/buttons/buttons_rp2xxx_simple.c
      ${CMAKE_SOURCE_DIR}/src/drivers/buttons/buttons_rp2xxx_simple.h
      ${CMAKE_SOURCE_DIR}/src/drivers/eeprom/eeprom_rp2xxx_emulated.c
      ${CMAKE_SOURCE_DIR}/src/drivers/eeprom/eeprom_rp2xxx_emulated.h
      ${CMAKE_SOURCE_DIR}/src/drivers/flash/flash_rp2xxx_internal.c
      ${CMAKE_SOURCE_DIR}/src/drivers/flash/flash_rp2xxx_internal.h
      ${CMAKE_SOURCE_DIR}/src/drivers/flash/flash_rp2xxx_internal.S
      ${CMAKE_SOURCE_DIR}/src/drivers/ir/ir_pico_pio.c # TODO Review
      ${CMAKE_SOURCE_DIR}/src/drivers/ir/ir_pico_pio.h # TODO Review
      ${CMAKE_SOURCE_DIR}/src/drivers/power/power_rp2xxx_ws.c
      ${CMAKE_SOURCE_DIR}/src/drivers/power/power_rp2xxx_ws.c
      ${CMAKE_SOURCE_DIR}/src/drivers/screen/screen_rp2xxx_gc9a01a_lvgl.c
      ${CMAKE_SOURCE_DIR}/src/drivers/screen/screen_rp2xxx_gc9a01a_lvgl.h
      ${CMAKE_SOURCE_DIR}/src/globals/picowalker_background_240x240_lvgl.c
      ${CMAKE_SOURCE_DIR}/src/globals/picowalker_background_240x240_lvgl.h
      ${CMAKE_SOURCE_DIR}/src/drivers/extras_pico.S
      ${CMAKE_SOURCE_DIR}/src/drivers/extras_pico.h
      ${CMAKE_SOURCE_DIR}/src/drivers/timer_pico.c
<<<<<<< HEAD
      ${CMAKE_SOURCE_DIR}/src/drivers/onboard_log.c
      ${CMAKE_SOURCE_DIR}/src/drivers/onboard_log.h
      ${CMAKE_SOURCE_DIR}/src/msc_disk.c
      ${CMAKE_SOURCE_DIR}/src/msc_disk.h
      ${CMAKE_SOURCE_DIR}/src/usb_descriptors.c
      ${CMAKE_SOURCE_DIR}/src/tusb_config.h
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
      ${CMAKE_SOURCE_DIR}/src/picowalker-defs.h
  )

  # Link these libs together
<<<<<<< HEAD
  target_link_libraries(picowalker_rp2350touchlcd128
=======
  target_link_libraries(picowalker
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
      WaveShare
      GC9A01A
      CST816S
      QMI8658
      lvgl
      pico_stdlib
<<<<<<< HEAD
      pico_aon_timer
=======
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
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
      ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
  )
else()
<<<<<<< HEAD
  add_executable(picowalker_rp2350touchlcd128
=======
  add_executable(picowalker
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
      ${CMAKE_SOURCE_DIR}/src/main.c
      ${CMAKE_SOURCE_DIR}/src/drivers/screen/screen_rp2xxx_gc9a01a.c
      ${CMAKE_SOURCE_DIR}/src/drivers/screen/screen_rp2xxx_gc9a01a.h
      ${CMAKE_SOURCE_DIR}/src/drivers/ir/ir_dummy.c
      ${CMAKE_SOURCE_DIR}/src/drivers/ir/ir_dummy.h
      ${CMAKE_SOURCE_DIR}/src/drivers/buttons/buttons_dummy.c
      ${CMAKE_SOURCE_DIR}/src/drivers/buttons/buttons_dummy.h
      ${CMAKE_SOURCE_DIR}/src/drivers/eeprom/eeprom_pico_ram.S
      ${CMAKE_SOURCE_DIR}/src/drivers/eeprom/eeprom_pico_ram.c
      ${CMAKE_SOURCE_DIR}/src/drivers/eeprom/eeprom_pico_ram.h
      ${CMAKE_SOURCE_DIR}/src/drivers/timer_pico.c
      ${CMAKE_SOURCE_DIR}/src/drivers/extras_pico.S
      ${CMAKE_SOURCE_DIR}/src/drivers/extras_pico.h
      ${CMAKE_SOURCE_DIR}/src/drivers/flash/flash_pico_internal.c
      ${CMAKE_SOURCE_DIR}/src/drivers/flash/flash_pico_internal.S
      ${CMAKE_SOURCE_DIR}/src/drivers/flash/flash_pico_internal.h
      #${CMAKE_SOURCE_DIR}/src/drivers/battery_pico_bq25628e.c
      #${CMAKE_SOURCE_DIR}/src/drivers/battery_pico_bq25628e.h
      #${CMAKE_SOURCE_DIR}/src/drivers/gpio_interrupts_pico.c
      #${CMAKE_SOURCE_DIR}/src/drivers/gpio_interrupts_pico.h
      ${CMAKE_SOURCE_DIR}/src/drivers/power/power_pico.c
      ${CMAKE_SOURCE_DIR}/src/drivers/power/power_pico.h
      ${CMAKE_SOURCE_DIR}/src/picowalker-defs.h
  )
  ## Link these libs together
<<<<<<< HEAD
  target_link_libraries(picowalker_rp2350touchlcd128
    pico_stdlib
    pico_aon_timer
    hardware_sleep
    hardware_spi
    hardware_i2c
    hardware_pio
    hardware_timer
    hardware_pwm
=======
  target_link_libraries(picowalker
      pico_stdlib
      hardware_sleep
      hardware_spi
      hardware_i2c
      hardware_pio
      hardware_timer
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
      ${CMAKE_SOURCE_DIR}/lib/libpicowalker-core.a
  )
endif()

<<<<<<< HEAD
### Add src/ as a header location for tinyusb
target_include_directories(picowalker_rp2350touchlcd128 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

### Enable usb, uart etc for stdio
#pico_enable_stdio_usb(picowalker_rp2350touchlcd128 1)
target_compile_definitions(picowalker_rp2350touchlcd128 PRIVATE
    PICO_DEFAULT_UART=0
    PICO_DEFAULT_UART_TX_PIN=28
    PICO_DEFAULT_UART_RX_PIN=29
)
pico_enable_stdio_uart(picowalker_rp2350touchlcd128 1)

### Add outputs other than .elf file
pico_add_extra_outputs(picowalker_rp2350touchlcd128)
=======
### Enable usb, uart etc for stdio
pico_enable_stdio_usb(picowalker 1)
# pico_enable_stdio_uart(picowalker 0)

### Add outputs other than .elf file
pico_add_extra_outputs(picowalker)
>>>>>>> 97250e6 (RP2350TouchLCD128 Working)
