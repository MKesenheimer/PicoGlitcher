cmake_minimum_required(VERSION 3.13)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
project(glitcher VERSION 1.0.0)

pico_sdk_init()

add_executable(glitcher
               glitcher.cpp)

pico_generate_pio_header(glitcher ${CMAKE_CURRENT_LIST_DIR}/pdnd/pio/spi.pio)
pico_generate_pio_header(glitcher ${CMAKE_CURRENT_LIST_DIR}/pdnd/pio/i2c.pio)
pico_generate_pio_header(glitcher ${CMAKE_CURRENT_LIST_DIR}/pdnd/pio/uart_rx.pio)
pico_generate_pio_header(glitcher ${CMAKE_CURRENT_LIST_DIR}/pdnd/pio/uart_tx.pio)


target_sources(glitcher PRIVATE
        glitcher.cpp

        # PIO components
        pdnd/pio/pio_spi.c
        pdnd/pio/pio_i2c.c

        # PDND source
        pdnd/pdnd.c
        pdnd/pdnd_spi.c

        # Display support
        pdnd/pdnd_display.c
        pdnd/display/FreeSans5pt7b.c
        pdnd/display/FreeSans9pt7b.c
        pdnd/display/font_renderer.c
        pdnd/display/ssd1306.c
        )

# todo: remove unecessary libraries
target_link_libraries(glitcher
                      pico_stdlib
                      pico_multicore
                      hardware_gpio
                      hardware_i2c
                      hardware_adc
                      hardware_pio)

# enable/disable the outputs
pico_enable_stdio_usb(glitcher 1)
pico_enable_stdio_uart(glitcher 1)
pico_add_extra_outputs(glitcher)
