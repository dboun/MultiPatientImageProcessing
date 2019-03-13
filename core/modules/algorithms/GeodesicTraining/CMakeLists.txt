# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

find_package(Qt5 COMPONENTS Core)
set(CMAKE_AUTOMOC ON)

add_subdirectory( 
  GeodesicTraining_external
)

set( CPP_FILES
  GeodesicTrainingModule.cpp
  GeodesicTrainingQt.cpp
  GeodesicTrainingQtOnly.cpp
) 
set( H_FILES
  GeodesicTrainingModule.h
  GeodesicTrainingQt.h
  GeodesicTrainingQtOnly.h
) 
set( UI_FILES

) 
set( MOC_H_FILES
  GeodesicTrainingModule.h
  GeodesicTrainingQt.h
  GeodesicTrainingQtOnly.h
) 


# Create the library
add_library( ${MODULE_NAME} STATIC
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES}
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  GeodesicTrainingSegmentation
  Qt5::Core
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)