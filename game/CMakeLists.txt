cmake_minimum_required(VERSION 3.20)
project(GameDLL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Source67 headers
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add source files
file(GLOB_RECURSE GAME_SOURCES "src/**/*.cpp")

# Create DLL
add_library(Game SHARED ${GAME_SOURCES})

# Link to Source67 (headers)
target_include_directories(Game PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Output to Release/Debug subdirectory
set_target_properties(Game PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
