#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RmlUi::Core" for configuration ""
set_property(TARGET RmlUi::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(RmlUi::Core PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/librmlui.so"
  IMPORTED_SONAME_NOCONFIG "librmlui.so"
  )

list(APPEND _cmake_import_check_targets RmlUi::Core )
list(APPEND _cmake_import_check_files_for_RmlUi::Core "${_IMPORT_PREFIX}/lib64/librmlui.so" )

# Import target "RmlUi::Debugger" for configuration ""
set_property(TARGET RmlUi::Debugger APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(RmlUi::Debugger PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/librmlui_debugger.so"
  IMPORTED_SONAME_NOCONFIG "librmlui_debugger.so"
  )

list(APPEND _cmake_import_check_targets RmlUi::Debugger )
list(APPEND _cmake_import_check_files_for_RmlUi::Debugger "${_IMPORT_PREFIX}/lib64/librmlui_debugger.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
