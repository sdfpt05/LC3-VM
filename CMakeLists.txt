# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(LC3_VM VERSION 1.0)

# Set C Standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Specify the source files
set(SOURCES
    src/lc3_core.c
    src/lc3_exec.c
    src/lc3_instructions.c
    src/lc3_traps.c
    src/main.c
)

# Include directories
include_directories(src)

# Create the executable
add_executable(lc3_vm ${SOURCES})

# Specify any required libraries (if needed)
# target_link_libraries(lc3_vm <library>)

# Optional: Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

