project( MpipBase )

find_package( Qt5 COMPONENTS Core Gui Widgets REQUIRED )

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/*.h") 

add_library( ${MODULE_NAME} 
  ${SOURCES}
  ${HEADERS}
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries( MpipBase
  Qt::Core
  Qt::Gui
  Qt::Widgets
)
