cmake_minimum_required(VERSION 3.1.0)

project( MPIP )

find_package(VTK)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})


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

set(CPP_FILES
  mainwindow.cpp
  main.cpp
  Scheduler.cpp
  ApplicationBase.cpp
  ApplicationGeodesicTrainingSegmentation.cpp
  )

set(MOC_H_FILES
  mainwindow.h
  Scheduler.h
  ApplicationBase.h
  ApplicationGeodesicTrainingSegmentation.h
  )

set(UI_FILES
  mainwindow.ui
)

set(H_FILES
  mainwindow.h
  Scheduler.h
  ApplicationBase.h
  ApplicationGeodesicTrainingSegmentation.h
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
target_link_libraries(MPIP ${VTK_LIBRARIES}  ${ITK_LIBRARIES} ${OpenCV_LIBRARIES} 
  ${APP_LIBRARIES} )


#target_link_libraries( MPIP PRIVATE 
 
#)
