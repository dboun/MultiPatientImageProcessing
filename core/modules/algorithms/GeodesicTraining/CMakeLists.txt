# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

#find_package(Qt5 COMPONENTS Core)
#set(CMAKE_AUTOMOC ON)

add_subdirectory( 
  GeodesicTraining_external
)

# Create the library
add_library( ${MODULE_NAME} STATIC
  ModuleGeodesicTraining.cpp
  GeodesicTrainingQt.cpp
  GeodesicTrainingQtOnly.cpp
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  GeodesicTrainingSegmentation
  #Qt5::Core
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)