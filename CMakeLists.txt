cmake_minimum_required(VERSION 3.1.0)

set( PROJECT_NAME MPIP )

project( ${PROJECT_NAME} VERSION 0.1.0 )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp" )

include( cmake/dependencies.cmake )

add_subdirectory( core )
add_definitions( ${CORE_DEFINITIONS_TO_ADD} )
remove_definitions( ${CORE_DEFINITIONS_TO_REMOVE} )

add_subdirectory( applications )
add_definitions( ${APPLICATION_DEFINITIONS_TO_ADD} )
remove_definitions( ${APPLICATION_DEFINITIONS_TO_REMOVE} )

# To find all files 
# TODO: replace with target_include_directories in subprojects
include_directories( 
  ${APP_INCLUDE_DIRS} 
  ${CORE_INCLUDE_DIRS}
)

include( cmake/files.cmake )

set( CMAKE_AUTOMOC ON )

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_executable( MPIP ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} )
qt5_use_modules( MPIP Core Gui Widgets )
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

target_link_libraries( MPIP 
  MpipCore
  ${APP_LIBRARIES}
)

# Installer
include( cmake/cpack.cmake )
