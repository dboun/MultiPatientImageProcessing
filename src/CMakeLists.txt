# Find Qt
find_package( Qt5 COMPONENTS Core Gui Widgets REQUIRED )

include( cmake/files.cmake )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

# Make terminal not pop up on application start
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Create the executable
add_executable( ${PROJECT_NAME} 
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} 
  ${RESOURCE_FILE}
)

set_target_properties( ${PROJECT_NAME} 
  PROPERTIES OUTPUT_NAME "MLL Semi-Automatic Segmentation"
)

qt5_use_modules( ${PROJECT_NAME} Core Gui Widgets )
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

target_link_libraries( ${PROJECT_NAME} 
  MpipBase
  MpipModules
)

target_include_directories( ${PROJECT_NAME} PUBLIC
  ${CURRENT_LIST_DIR}
)

# Installer
include( cmake/cpack.cmake )