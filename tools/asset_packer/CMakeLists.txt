cmake_minimum_required(VERSION 3.20)
project(AssetPacker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(ASSET_PACKER_SOURCES
    main.cpp
    AssetPacker.cpp
    AssetPacker.h
    AssetPackerTypes.h
)

# Create executable
add_executable(asset_packer ${ASSET_PACKER_SOURCES})

# Set output directory
set_target_properties(asset_packer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Install target
install(TARGETS asset_packer
    RUNTIME DESTINATION bin
)
