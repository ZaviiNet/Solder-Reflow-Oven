# CMakeLists.txt for CLion integration with PlatformIO
# This file enables CLion's code completion and navigation features
# while using PlatformIO as the actual build system

cmake_minimum_required(VERSION 3.13)
project(Solder_Reflow_Oven_IO C CXX ASM)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Include PlatformIO generated build files
include(CMakeListsPrivate.txt OPTIONAL)

# If CLion can't find PlatformIO includes, uncomment and adjust these paths:
# include_directories("${CMAKE_CURRENT_LIST_DIR}/.pio/libdeps/nodemcuv2")
# include_directories("${CMAKE_CURRENT_LIST_DIR}/.pio/libdeps/pico")

# Add source files for IDE code completion
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
    "src/*.h"
)

# Add include directories
include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

# Create a dummy target for CLion to analyze
# This won't actually build - PlatformIO handles the real build
add_executable(${PROJECT_NAME} ${SOURCES})

# Note: Use PlatformIO for actual building, uploading, and monitoring
# In CLion terminal: pio run, pio run -t upload, pio device monitor

