# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(LC3_VM VERSION 1.0)

# Set C Standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Optional: Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
endif()

# Specify the source files
set(SOURCES
    src/lc3_core.c
    src/lc3_instructions.c
    src/lc3_traps.c
    src/main.c
)

# Include directories
include_directories(src)

# Create the executable
add_executable(lc3_vm ${SOURCES})

# Check if we need to link with specific libraries on different platforms
if(UNIX AND NOT APPLE)
    target_link_libraries(lc3_vm m)  # Link math library on Linux
endif()

# Optional: Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install target
install(TARGETS lc3_vm DESTINATION bin)

# Add a simple test target
enable_testing()
add_test(NAME basic_test COMMAND lc3_vm --help)

# Print configuration summary
message(STATUS "Configuration:")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")

