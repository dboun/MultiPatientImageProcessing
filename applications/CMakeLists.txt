# Modular applications
project( MpipApplications )

unset( APP_SOURCES )
unset( APP_HEADERS )
unset( APP_UI_FILES )
unset( APP_INCLUDE_DIRS )

set( CMAKE_AUTOMOC ON )

find_package( Qt5 COMPONENTS Widgets REQUIRED )

include( ApplicationGeodesicTrainingSegmentation.cmake )

set( CPP_FILES ${APP_SOURCES} )
set( MOC_H_FILES ${APP_HEADERS} )
set( UI_FILES ${APP_UI_FILES} )
set( H_FILES ${APP_HEADERS} )

add_library( MpipApplications ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} )
qt5_use_modules( MpipApplications Core Gui Widgets )
SET( CMAKE_INCLUDE_CURRENT_DIR ON )

target_link_libraries( MpipApplications
  ${APP_LIBRARIES}
)

target_include_directories( MpipApplications PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${APP_INCLUDE_DIRS}
)

# Definitions 
set( APPLICATION_DEFINITIONS_TO_ADD ${APPLICATION_DEFINITIONS_TO_ADD} CACHE INTERNAL "")
set( APPLICATION_DEFINITIONS_TO_REMOVE ${APPLICATION_DEFINITIONS_TO_REMOVE} CACHE INTERNAL "")