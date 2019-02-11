cmake_minimum_required(VERSION 3.1.0)

project( MPIP )

find_package( VTK )
include( ${VTK_USE_FILE} )

find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

add_subdirectory( Applications )

include_directories( ${APP_INCLUDE_DIRS} )

set( CPP_FILES
  mainwindow.cpp
  main.cpp
  Scheduler.cpp
  ApplicationBase.cpp
  ${APP_SOURCES}
)

set( MOC_H_FILES
  mainwindow.h
  Scheduler.h
  ApplicationBase.h
  ${APP_HEADERS}
)

set( UI_FILES
  mainwindow.ui
)

set( H_FILES
  mainwindow.h
  Scheduler.h
  ApplicationBase.h
  ${APP_HEADERS}
)
 
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set( CMAKE_AUTOMOC ON )
  
find_package( Qt5 COMPONENTS Widgets REQUIRED )
qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_executable( MPIP ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} )
qt5_use_modules( MPIP Core Gui Widgets )

if (BUILD_GEODESIC_TRAINING)

target_link_libraries( MPIP 
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${OpenCV_LIBRARIES} 
  ${APP_LIBRARIES})

else(BUILD_GEODESIC_TRAINING)

target_link_libraries( MPIP 
  ${VTK_LIBRARIES}
  #${ITK_LIBRARIES}
  #${OpenCV_LIBRARIES} 
)
endif(BUILD_GEODESIC_TRAINING)