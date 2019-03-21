cmake_minimum_required(VERSION 3.1.0)

set( PROJECT_NAME MPIP )
set( PROJECT_VERSION 0.1.0 )

project( ${PROJECT_NAME} VERSION ${PROJECT_VERSION} )

# Handles everything related to the core
include( core/cmake/master.cmake )

# Handles everything related to the application
add_subdirectory( src )

# Installer
#include( cpack.cmake )
