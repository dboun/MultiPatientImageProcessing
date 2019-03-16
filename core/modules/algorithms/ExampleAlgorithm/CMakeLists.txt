# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

# Create the library
add_library( ${MODULE_NAME} SHARED
  random.cpp 
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)
