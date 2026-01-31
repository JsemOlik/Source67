#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Lua::lua_shared" for configuration "Debug"
set_property(TARGET Lua::lua_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Lua::lua_shared PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/lua_shared.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/lua_shared.dll"
  )

list(APPEND _cmake_import_check_targets Lua::lua_shared )
list(APPEND _cmake_import_check_files_for_Lua::lua_shared "${_IMPORT_PREFIX}/lib/lua_shared.lib" "${_IMPORT_PREFIX}/bin/lua_shared.dll" )

# Import target "Lua::lua_static" for configuration "Debug"
set_property(TARGET Lua::lua_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Lua::lua_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/lua_static.lib"
  )

list(APPEND _cmake_import_check_targets Lua::lua_static )
list(APPEND _cmake_import_check_files_for_Lua::lua_static "${_IMPORT_PREFIX}/lib/lua_static.lib" )

# Import target "Lua::luac" for configuration "Debug"
set_property(TARGET Lua::luac APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Lua::luac PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/luac.exe"
  )

list(APPEND _cmake_import_check_targets Lua::luac )
list(APPEND _cmake_import_check_files_for_Lua::luac "${_IMPORT_PREFIX}/bin/luac.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
