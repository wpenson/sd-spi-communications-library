cmake_minimum_required(VERSION 3.5)
project(sd_spi_emulator)

set(SOURCE_FILES
	sd_spi_emulator.c
    ../sd_spi.h
    ../sd_spi_commands.h
    ../sd_spi_info.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})