cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
cmake_policy(SET CMP0135 NEW)
project(Source67)

# Trigger re-scan of src/ directory

set(CMAKE_CXX_STANDARD 20)
# Nudge to pick up Framebuffer.cpp
# Nudge to pick up SceneSerializer.cpp
# Nudge to pick up PlatformUtils.cpp
# Nudge to pick up ContentBrowserPanel.cpp
# Nudge to pick up PlayerController.cpp
# Nudge to pick up Window.mm
# Nudge to pick up Scene.cpp
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force dynamic CRT on Windows to match Jolt and other libs if USE_STATIC_CRT is OFF
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Directory Structure
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGuizmo
)

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
    GIT_TAG        v5.0.0
    SOURCE_SUBDIR  Build
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)

set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
set(GENERATE_DEBUG_SYMBOLS OFF CACHE BOOL "" FORCE)
set(UNITTESTS OFF CACHE BOOL "" FORCE)
set(SAMPLES OFF CACHE BOOL "" FORCE)
set(PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
# Disable warnings-as-errors for Jolt to avoid Windows SDK header warnings
set(CROSS_PLATFORM_DETERMINISTIC OFF CACHE BOOL "" FORCE)

# sol2 (C++ Lua binding)
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2
    GIT_TAG        v3.5.0
)

# Lua
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua
    GIT_TAG        v5.4.7
)

FetchContent_MakeAvailable(glfw glm spdlog jolt imgui json sol2 lua)

# Override /WX flag for Jolt target on MSVC to prevent Windows SDK warnings from failing the build
# Using /WX- explicitly disables warnings-as-errors and overrides any /WX set by Jolt's CMake
if(MSVC AND TARGET Jolt)
    target_compile_options(Jolt PRIVATE /WX-)
endif()

# GLAD - Using latest commit from master branch
# TODO: Pin to specific commit once a stable tag is available
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
# Added Skybox.cpp

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/**/*.cpp"
    "src/**/*.h"
    "vendor/**/*.cpp"
    "vendor/**/*.h"
)

if(APPLE)
    file(GLOB_RECURSE MAC_SOURCES "src/**/*.mm")
    list(APPEND SOURCES ${MAC_SOURCES})
endif()
list(APPEND SOURCES "main.cpp")
list(APPEND SOURCES "src/Renderer/Scene.cpp")

set(IMGUI_DIR ${imgui_SOURCE_DIR})
list(APPEND SOURCES 
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# On non-Apple platforms, compile .mm files as regular C++
if(NOT APPLE)
    foreach(source ${SOURCES})
        if(source MATCHES "\\.mm$")
            set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
        endif()
    endforeach()
endif()

add_executable(Source67 WIN32 ${SOURCES})

target_include_directories(Source67 PUBLIC 
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${jolt_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${sol2_SOURCE_DIR}/include
)

# Let's set up the executable

# Defaults for Editor Source67 target
target_link_libraries(Source67 PUBLIC 
    glfw 
    glm::glm
    spdlog::spdlog
    glad
    Jolt
    nlohmann_json::nlohmann_json
    lua_static
    sol2
)

# Compiler warnings
if(MSVC)
    target_compile_options(Source67 PRIVATE /W4)
else()
    target_compile_options(Source67 PRIVATE -Wall -Wextra -Wpedantic -Wno-changes-meaning)
endif()

# Sanitizers for debug builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(NOT MSVC)
        target_compile_options(Source67 PRIVATE -fsanitize=address,undefined)
        target_link_options(Source67 PRIVATE -fsanitize=address,undefined)
    endif()
endif()

if(MSVC)
    target_compile_definitions(Source67 PUBLIC 
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
    )
endif()

if(APPLE)
    target_link_libraries(Source67 PUBLIC "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Source67 PUBLIC OpenGL::GL)
endif()

# --- Runtime Executable Target ---

# logical copy of sources
set(RUNTIME_SOURCES ${SOURCES})

# Exclude Editor-specific files
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX "src/ImGui/Panels/.*")
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX "src/ImGui/ImGuiLayer\\.cpp")
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX "src/Game/Console/ConsolePanel\\.cpp")
list(FILTER RUNTIME_SOURCES EXCLUDE REGEX "vendor/ImGuizmo/.*")

add_executable(Source67-Runtime WIN32 ${RUNTIME_SOURCES})

target_compile_definitions(Source67-Runtime PUBLIC S67_RUNTIME)

target_include_directories(Source67-Runtime PUBLIC 
    ${INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${jolt_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${sol2_SOURCE_DIR}/include
)

target_link_libraries(Source67-Runtime PUBLIC 
    glfw 
    glm::glm
    spdlog::spdlog
    glad
    Jolt
    nlohmann_json::nlohmann_json
    lua_static
    sol2
)

if(MSVC)
    target_compile_options(Source67-Runtime PRIVATE /W4)
    target_compile_definitions(Source67-Runtime PUBLIC _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
else()
    target_compile_options(Source67-Runtime PRIVATE -Wall -Wextra -Wpedantic -Wno-changes-meaning)
endif()

if(APPLE)
    target_link_libraries(Source67-Runtime PUBLIC "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX)
    target_link_libraries(Source67-Runtime PUBLIC OpenGL::GL)
endif()

# Ensure Editor always builds the Runtime template
add_dependencies(Source67 Source67-Runtime)

# Copy assets to build directory
# Copy assets to build directory
# add_custom_command(TARGET Source67 POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_CURRENT_SOURCE_DIR}/assets
#     $<TARGET_FILE_DIR:Source67>/assets
# )
