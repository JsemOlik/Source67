# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-src")
  file(MAKE_DIRECTORY "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-build"
  "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix"
  "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix/tmp"
  "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix/src/jolt-populate-stamp"
  "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix/src"
  "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix/src/jolt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix/src/jolt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/olik/Desktop/Coding/Source67/cmake-build-debug/_deps/jolt-subbuild/jolt-populate-prefix/src/jolt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
