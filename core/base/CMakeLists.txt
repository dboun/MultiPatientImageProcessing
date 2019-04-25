project( MpipBase )

find_package( Qt5 COMPONENTS Core Gui Widgets REQUIRED )

set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h") 
file(GLOB_RECURSE UIFILES "*.ui")

qt5_wrap_ui( UI_Srcs ${UIFILES} )

add_library( MpipBase STATIC
  ${SOURCES}
  ${HEADERS}
  ${UIFILES}
)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

target_include_directories( MpipBase PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/algorithm
  ${PROJECT_SOURCE_DIR}/gui
  ${PROJECT_SOURCE_DIR}/gui/basic_elements
  ${PROJECT_SOURCE_DIR}/other
)

target_link_libraries( MpipBase
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)
