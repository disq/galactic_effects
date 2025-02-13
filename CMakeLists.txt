cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Default path to the SDK
if (NOT PICO_SDK_PATH)
  set(PICO_SDK_PATH "../pico-sdk")
endif ()

# Pull in Pico SDK (must be before project)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
project(firmware C CXX ASM)

pico_sdk_init()

add_executable(firmware main.c galactic.cpp galactic.h metaballs.c metaballs.h plasma.c plasma.h rotozoom.c rotozoom.h deform.c deform.h convert.c convert.h head.h)

pico_set_program_name(firmware "Galactic Effects")
pico_set_program_version(firmware "0.1")

# Enable usb output, disable uart output
pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 0)

# Add the standard library to the build
target_link_libraries(firmware pico_stdlib)

# Add any user requested libraries
target_link_libraries(firmware
        hardware_spi
        hardware_dma
        hagl
        hagl_hal
        pico_bootsel_via_double_reset
        )

pico_add_extra_outputs(firmware)

add_subdirectory(external/hagl)
add_subdirectory(external/hagl_galactic)

#
# If you have a large screen and run out of memory use
# the HAGL_HAL_PIXEL_SIZE=2 setting.
#
target_compile_definitions(firmware PRIVATE
#  HAGL_HAL_USE_SINGLE_BUFFER
  HAGL_HAL_USE_DOUBLE_BUFFER
#  HAGL_HAL_USE_TRIPLE_BUFFER
#  HAGL_HAL_USE_DMA
#  HAGL_HAL_DEBUG
#  HAGL_HAL_PIXEL_SIZE=2
  DISPLAY_WIDTH=212
  DISPLAY_HEIGHT=44
#  DISPLAY_HEIGHT=88
  GALACTIC_PIXEL_DIVISOR_X=2
  GALACTIC_PIXEL_DIVISOR_Y=2
#  GALACTIC_PIXEL_DIVISOR_Y=3
)
