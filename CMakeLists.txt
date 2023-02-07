cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_datascope C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(pico_datascope
        pico_datascope.c
        xbee.c
        xterm.c
        )

target_link_libraries(pico_datascope
        pico_stdlib              # for core functionality
        pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
        pico_multicore
        pico_unique_id 
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(pico_datascope 1)
pico_enable_stdio_uart(pico_datascope 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_datascope)

