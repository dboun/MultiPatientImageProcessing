# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

find_package( Qt5 COMPONENTS Widgets REQUIRED )

set(CMAKE_AUTOMOC ON)

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_subdirectory( 
  GeodesicTraining_external/GeodesicTrainingSegmentation 
)

# Setting the files of the module
set( CPP_FILES
  ModuleGeodesicTraining.cpp
)

set( MOC_H_FILES
  ModuleGeodesicTraining.h
)

set( UI_FILES
  
)

set( H_FILES
  ModuleGeodesicTraining.h
)

# Create the library
add_library( ${MODULE_NAME} SHARED
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} 
)

qt5_use_modules( ${MODULE_NAME} Widgets )

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  GeodesicTrainingSegmentation
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
