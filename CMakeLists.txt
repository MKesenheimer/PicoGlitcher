cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(glitcher VERSION 1.0.0)
add_executable(glitcher
               glitcher.cpp)

pico_sdk_init()

pico_enable_stdio_usb(glitcher 1)
pico_enable_stdio_uart(glitcher 1)
pico_add_extra_outputs(glitcher)

target_link_libraries(glitcher
                      pico_stdlib
                      hardware_gpio
                      hardware_i2c
                      hardware_adc)
