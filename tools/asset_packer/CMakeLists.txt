cmake_minimum_required(VERSION 3.20)
project(AssetPacker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(asset_packer main.cpp)

target_link_libraries(asset_packer PUBLIC spdlog::spdlog)
