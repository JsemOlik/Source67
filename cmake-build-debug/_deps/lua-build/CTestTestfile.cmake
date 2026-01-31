# CMake generated Testfile for 
# Source directory: C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/lua-src
# Build directory: C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/lua-build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(lua-testsuite "lua" "-e" "_U=true" "all.lua")
set_tests_properties(lua-testsuite PROPERTIES  WORKING_DIRECTORY "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/lua-src/lua--tests" _BACKTRACE_TRIPLES "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/lua-src/CMakeLists.txt;32;add_test;C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/lua-src/CMakeLists.txt;0;")
subdirs("lua-5.4.7")
