cmake_minimum_required(VERSION 3.5)
project(sd_spi)

set(SOURCE_FILES
    device/sd_spi.h
    sd_spi_commands.h
    sd_spi_info.h
    emulator/sd_spi_emulator.c
    emulator/sd_spi_emulator.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Expose sd_spi's public includes to other subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
        CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)