cmake_minimum_required(VERSION 3.15)
project(tiny_cbase C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/tiny_cbase.c
    test/cbase_demo.c
)

# Create executable
add_executable(tiny_cbase ${SOURCES})

# Compiler flags
if(MSVC)
    target_compile_options(tiny_cbase PRIVATE /W3 /O2)
else()
    target_compile_options(tiny_cbase PRIVATE -Wall -Wextra -O2)
endif()

# output directory
set_target_properties(tiny_cbase PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)