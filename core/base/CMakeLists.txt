project( MpipBase )

find_package( Qt5 COMPONENTS Core Gui Widgets REQUIRED )

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/*.h") 

add_library( MpipBase SHARED
  ${SOURCES}
  ${HEADERS}
)

target_include_directories( MpipBase PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/algorithm
  ${PROJECT_SOURCE_DIR}/gui
  ${PROJECT_SOURCE_DIR}/other
)

target_link_libraries( MpipBase
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)
