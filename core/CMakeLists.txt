PROJECT( MpipCore )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp" )
set( CMAKE_AUTOMOC ON )

include( cmake/files.cmake )
include( cmake/viewer.cmake )
  
find_package( Qt5 COMPONENTS Widgets REQUIRED )
qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_library( MpipCore ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} )
qt5_use_modules( MpipCore Core Gui Widgets )
SET( CMAKE_INCLUDE_CURRENT_DIR ON )

target_link_libraries( MpipCore 
  ${VIEWER_LIBRARIES_TO_LINK}
  ${APP_LIBRARIES}
)

target_include_directories( MpipCore PUBLIC
  ${APP_INCLUDE_DIRS}
)

set( CORE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" )