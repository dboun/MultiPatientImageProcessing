# Note: Project name is set by options.cmake
project( ${MODULE_NAME} )

find_package( Qt5 COMPONENTS Core Gui Widgets )
find_package( MITK REQUIRED )
set(CMAKE_AUTOMOC ON)

set( CPP_FILES
  MitkImageViewer.cpp
  CustomQmitkStdMultiWidget.cpp
) 
set( H_FILES
  MitkImageViewer.h
  CustomQmitkStdMultiWidget.h
) 
set( UI_FILES

) 
set( MOC_H_FILES
  MitkImageViewer.h
  CustomQmitkStdMultiWidget.h
) 

# Context for the CustomQmitkStdMultiWidget
set(module_srcs )
usFunctionGenerateModuleInit(module_srcs)

# Create the library
add_library( ${MODULE_NAME} STATIC
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES}
  ${module_srcs}
)

# Also for CustomQmitkStdMultiWidget
set_property(TARGET ${MODULE_NAME} 
  APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${MODULE_NAME}
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
  ${NEEDS_MODULES}

  Qt5::Core
  Qt5::Gui
  Qt5::Widgets

  MitkCore 
  MitkQtWidgets 
  MitkQtWidgetsExt
  MitkSegmentation
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)
