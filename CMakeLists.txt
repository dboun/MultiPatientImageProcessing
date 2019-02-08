cmake_minimum_required(VERSION 3.1.0)

project( MPIP )

find_package(VTK)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set(CPP_FILES
  mainwindow.cpp
  main.cpp
  Scheduler.cpp
  )

set(H_FILES
  mainwindow.h
  Scheduler.h
  )
  
set(MOC_H_FILES
  mainwindow.h
  Scheduler.h
  )

set(UI_FILES
  mainwindow.ui
  )
  
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(CMAKE_AUTOMOC ON)
  
find_package(Qt5 COMPONENTS Widgets REQUIRED)
qt5_wrap_ui(UI_Srcs ${UI_FILES})

add_executable(MPIP ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES})
qt5_use_modules(MPIP Core Gui Widgets)
target_link_libraries(MPIP ${VTK_LIBRARIES}  ${ITK_LIBRARIES})

#find_package( MITK REQUIRED )
find_package( OpenCV REQUIRED )

#Include( mitkFunctionCheckCompilerFlags )

add_subdirectory( Applications )

#message( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${APP_INCLUDE_DIRS} )

# For the executable
#mitk_create_executable( MPIP
#  DEPENDS MitkQtWidgetsExt
#  PACKAGE_DEPENDS ITK|ITKCurvatureFlow+ITKRegionGrowing
#)

target_link_libraries( MPIP PRIVATE 
  ${OpenCV_LIBRARIES} 
  ${APP_LIBRARIES} 
)
