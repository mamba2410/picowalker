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
)

## Enable usb, uart etc for stdio
pico_enable_stdio_usb(picowalker 1)
pico_enable_stdio_uart(picowalker 0)

## Add outputs other than .elf file
pico_add_extra_outputs(picowalker)

## Link these libs together
target_link_libraries(picowalker
	pico_stdlib
)
