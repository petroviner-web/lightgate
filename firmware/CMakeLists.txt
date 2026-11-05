cmake_minimum_required(VERSION 3.13)

# Requires the Raspberry Pi Pico SDK. Set PICO_SDK_PATH to its location.
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(sprint_timer C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(sprint_timer
    src/main.c
    src/hal_pico.c
    src/sprint_fsm.c
)

target_include_directories(sprint_timer PRIVATE include)
target_link_libraries(sprint_timer pico_stdlib hardware_irq)

# Send printf output over USB serial (the backend reads it from a COM port).
pico_enable_stdio_usb(sprint_timer 1)
pico_enable_stdio_uart(sprint_timer 0)

pico_add_extra_outputs(sprint_timer)
