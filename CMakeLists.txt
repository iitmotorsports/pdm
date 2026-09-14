cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME pdm)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        PDM/Src/pdm.c
        PDM/Src/pdm.c
        PDM/Inc/pdm.h
        PDM/Src/pdm_status_led.c
        PDM/Inc/pdm_status_led.h)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add CANopenNode sources (excluding example/ folder)
file(GLOB_RECURSE CANOPENNODE_SOURCES
    "CANopenNode/*.c"
)
# Remove example folder files from the list
list(FILTER CANOPENNODE_SOURCES EXCLUDE REGEX ".*example/.*")

# Add CANopenNodeSTM32 sources
file(GLOB_RECURSE CANOPENNODESTM32_SOURCES
    "CANopenNode_STM32/*.c"
)

file(GLOB_RECURSE PDM_CODE_SOURCES
        "PDM/Src/*.c"
        "PDM/Inc/*.h"
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${PDM_CODE_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/PDM
    ${CMAKE_CURRENT_SOURCE_DIR}/PDM/Src
    ${CMAKE_CURRENT_SOURCE_DIR}/PDM/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
