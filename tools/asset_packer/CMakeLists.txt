cmake_minimum_required(VERSION 3.20)
project(AssetPacker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files for library
set(ASSET_PACKER_LIB_SOURCES
    AssetPacker.cpp
    AssetPacker.h
    AssetPackerTypes.h
)

# Source files for executable
set(ASSET_PACKER_EXE_SOURCES
    main.cpp
)

# Create library for use by BuildSystem
add_library(AssetPackerLib STATIC ${ASSET_PACKER_LIB_SOURCES})
target_include_directories(AssetPackerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Create executable
add_executable(asset_packer ${ASSET_PACKER_EXE_SOURCES})
target_link_libraries(asset_packer PRIVATE AssetPackerLib)

# Set output directory
set_target_properties(asset_packer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Install targets
install(TARGETS asset_packer AssetPackerLib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
