cmake_minimum_required(VERSION 3.1.0)

set( PROJECT_NAME MPIP )

project( ${PROJECT_NAME} VERSION 0.1.0 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package( VTK )
include( ${VTK_USE_FILE} )

add_subdirectory( Applications )

# Find all files
include( files.cmake )
 
include_directories( ${APP_INCLUDE_DIRS} )

set( CMAKE_AUTOMOC ON )
  
find_package( Qt5 COMPONENTS Widgets REQUIRED )
qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_executable( MPIP ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} )
qt5_use_modules( MPIP Core Gui Widgets )
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

target_link_libraries( MPIP 
  ${VTK_LIBRARIES}
  ${APP_LIBRARIES}
)

# Installer
include( cpack.cmake )