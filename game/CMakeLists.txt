cmake_minimum_required(VERSION 3.20)
project(GameDLL CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
file(GLOB_RECURSE GAME_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# Create shared library (DLL)
add_library(Game SHARED ${GAME_SOURCES})

# Add compile definition for DLL export
target_compile_definitions(Game PRIVATE GAME_DLL_EXPORT)

# Include Source67 headers (for production use)
# Uncomment when linking against Source67 engine
# target_include_directories(Game PRIVATE 
#     ${CMAKE_SOURCE_DIR}/../src
# )

# Set output directories
set_target_properties(Game PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(Game PROPERTIES
        OUTPUT_NAME "Game"
        SUFFIX ".dll"
    )
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    set_target_properties(Game PROPERTIES
        OUTPUT_NAME "Game"
        PREFIX "lib"
        SUFFIX ".so"
    )
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(Game PROPERTIES
        OUTPUT_NAME "Game"
        PREFIX "lib"
        SUFFIX ".dylib"
    )
endif()

# Install target
install(TARGETS Game
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)
