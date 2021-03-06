# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

find_package( Qt5 COMPONENTS Core Gui Widgets )
find_package( MITK REQUIRED )
set(CMAKE_AUTOMOC ON)

set( CPP_FILES
  CustomMitkDataStorage.cpp
) 
set( H_FILES
  CustomMitkDataStorage.h
) 
set( UI_FILES
  
) 
set( MOC_H_FILES
  CustomMitkDataStorage.h
) 

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

# Create the library
add_library( ${MODULE_NAME} STATIC
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES}
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  ${NEEDS_MODULES}  

  Qt5::Core
  Qt5::Gui
  Qt5::Widgets

  MitkCore
  MitkMultilabel
  MitkSegmentation
)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)

#add_subdirectory(test)
