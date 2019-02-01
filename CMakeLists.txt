cmake_minimum_required(VERSION 3.1.0)

project( MPIP )

# Dependencies
find_package( OpenCV REQUIRED )
find_package( MITK   REQUIRED )

Include( mitkFunctionCheckCompilerFlags )

add_subdirectory( Applications )

#message( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${APP_INCLUDE_DIRS} )

# For the executable
mitk_create_executable( MPIP
  DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS ITK|ITKCurvatureFlow+ITKRegionGrowing
)

target_link_libraries( MPIP PRIVATE 
  ${OpenCV_LIBRARIES} 
  ${APP_LIBRARIES} 
)