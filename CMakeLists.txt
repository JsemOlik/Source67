cmake_minimum_required(VERSION 3.20)
project(Source67)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directory Structure
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)

# Jolt Physics
FetchContent_Declare(
    jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG        master
    SOURCE_SUBDIR  Build
)
set(USE_STATIC_CRT ON CACHE BOOL "" FORCE)
set(GENERATE_DEBUG_SYMBOLS OFF CACHE BOOL "" FORCE)
set(UNITTESTS OFF CACHE BOOL "" FORCE)
set(SAMPLES OFF CACHE BOOL "" FORCE)
set(PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw glm spdlog jolt)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/phoenix-engine/glad-4.1-core.git
    GIT_TAG master
)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    add_library(glad STATIC ${glad_SOURCE_DIR}/src/glad.c)
    target_include_directories(glad PUBLIC ${glad_SOURCE_DIR}/include)
endif()

# Let's set up the executable

# Source files - globbing is used for convenience, but requires re-configuring when adding new files
# (Note: Touching this file triggers a re-scan of the src/ and vendor/ directories)

file(GLOB_RECURSE SOURCES 
    "src/**/*.cpp"
    "src/**/*.h"
    "vendor/**/*.cpp"
    "vendor/**/*.h"
)
list(APPEND SOURCES "main.cpp")

add_executable(Source67 ${SOURCES})

target_include_directories(Source67 PUBLIC 
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
    ${jolt_SOURCE_DIR}
)

# Let's set up the executable

target_link_libraries(Source67 PUBLIC 
    glfw 
    glm::glm
    spdlog::spdlog
    glad
    Jolt
)

if(APPLE)
    target_link_libraries(Source67 PUBLIC "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Copy assets to build directory
add_custom_command(TARGET Source67 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Source67>/assets
)
