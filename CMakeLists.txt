cmake_minimum_required(VERSION 3.1.0)

project( MPIP )

option(BUILD_WITH_MITK "Enable building with MITK viewer/tools" ON)

if(BUILD_WITH_MITK)
  find_package( MITK REQUIRED )
endif(BUILD_WITH_MITK)

find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

add_subdirectory( Applications )

include_directories( ${APP_INCLUDE_DIRS} )

set( CPP_FILES
  mainwindow.cpp
  main.cpp
  Scheduler.cpp
  ApplicationBase.cpp
  MpipMitkViewer.cpp
  ${APP_SOURCES}
)

set( MOC_H_FILES
  mainwindow.h
  Scheduler.h
  ApplicationBase.h
  MpipMitkViewer.h
  ${APP_HEADERS}
)

set( UI_FILES
  mainwindow.ui
)

set( H_FILES
  mainwindow.h
  Scheduler.h
  ApplicationBase.h
  MpipMitkViewer.h
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

if( BUILD_WITH_MITK )

  add_definitions(-DBUILD_WITH_MITK)
  message(STATUS "Linking with MITK")

  set( LIBRARIES_TO_LINK 
    MitkCore MitkMultilabel MitkQtWidgets MitkQtWidgetsExt
  )

else( BUILD_WITH_MITK )

  remove_definitions(-DBUILD_WITH_MITK)
  message(STATUS "Linking without MITK")

  find_package( VTK REQUIRED )
  include( ${VTK_USE_FILE} )

  set( LIBRARIES_TO_LINK
    ${VTK_LIBRARIES} 
  )

endif( BUILD_WITH_MITK )

target_link_libraries( MPIP 
  ${LIBRARIES_TO_LINK}
  ${APP_LIBRARIES}
)

#if (BUILD_GEODESIC_TRAINING)

  #message(STATUS "Linking with Geodesic Training")

  #target_link_libraries( MPIP 
    #${LIBRARIES_TO_LINK}
    #${ITK_LIBRARIES}
    #${OpenCV_LIBRARIES} 
    #${APP_LIBRARIES}
  #)

#else(BUILD_GEODESIC_TRAINING)

#  target_link_libraries( MPIP 
#    ${LIBRARIES_TO_LINK}
#  )

#endif(BUILD_GEODESIC_TRAINING)