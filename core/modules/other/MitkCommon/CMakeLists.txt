# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

find_package( Qt5 COMPONENTS Core )
find_package( MITK REQUIRED )
set(CMAKE_AUTOMOC ON)

set( CPP_FILES
  MitkDataManager.cpp
) 
set( H_FILES
  MitkDataManager.h
) 
set( UI_FILES

) 
set( MOC_H_FILES
  MitkDataManager.h
) 


# Create the library
add_library( ${MODULE_NAME} STATIC
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES}
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  Qt5::Core
  MitkCore 
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)
