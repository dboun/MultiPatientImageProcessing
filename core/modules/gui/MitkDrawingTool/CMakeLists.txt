# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

find_package( Qt5 COMPONENTS Core Gui Widgets )
find_package( MITK REQUIRED )
set(CMAKE_AUTOMOC ON)

set( CPP_FILES
  MitkDrawingTool.cpp
) 
set( H_FILES
  MitkDrawingTool.h
) 
set( UI_FILES
  MitkDrawingTool.ui
) 
set( MOC_H_FILES
  MitkDrawingTool.h
) 

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

# Create the library
add_library( ${MODULE_NAME} STATIC
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES}
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  MitkImageViewer # The module (not from MITK directly)

  Qt5::Core
  Qt5::Gui
  Qt5::Widgets

  MitkCore 
  MitkQtWidgets 
  MitkQtWidgetsExt
  MitkMultilabel
  MitkSegmentation
  MitkSegmentationUI
)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)
