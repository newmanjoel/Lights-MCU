# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD wiznet_w5100s_evb_pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Lights-MCU C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Define include and source directories
set(LIGHTS_MCU_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(LIGHTS_MCU_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

# Collect all .cpp files in src/
file(GLOB_RECURSE LIGHTS_MCU_SOURCES
    "${LIGHTS_MCU_SRC_DIR}/*.cpp"
)

message(STATUS "LIGHTS_MCU_SOURCES = ${LIGHTS_MCU_SOURCES}")

# Add executable. Default name is the project name, version 0.1

add_executable(Lights-MCU 
    Lights-MCU.cpp 
    ${LIGHTS_MCU_SOURCES}
)

target_include_directories(Lights-MCU PRIVATE 
    ${LIGHTS_MCU_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}
)

pico_set_program_name(Lights-MCU "Lights-MCU")
pico_set_program_version(Lights-MCU "0.1")

# Generate PIO header
pico_generate_pio_header(Lights-MCU ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Lights-MCU 1)
pico_enable_stdio_usb(Lights-MCU 0)

# Add the standard library to the build
target_link_libraries(Lights-MCU
        pico_stdlib
        pico_multicore)

# Add the standard include files to the build
target_include_directories(Lights-MCU PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(Lights-MCU 
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        )

pico_add_extra_outputs(Lights-MCU)

