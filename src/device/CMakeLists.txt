cmake_minimum_required(VERSION 3.5)
project(sd_spi_device)

set(SOURCE_FILES
	sd_spi_platform_dependencies.c
	sd_spi_platform_dependencies.h
	sd_spi.c
    ../sd_spi.h
    ../sd_spi_commands.h
    ../sd_spi_info.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Expose sd_spi_device's public includes to other subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
        CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)