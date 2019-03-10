project( MpipCore )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp" )
set( CMAKE_AUTOMOC ON )

include( cmake/files.cmake )
include( cmake/mitk.cmake )
  
find_package( Qt5 COMPONENTS Widgets REQUIRED )
qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_library( MpipCore ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} )
qt5_use_modules( MpipCore Core Gui Widgets )

target_link_libraries( MpipCore 
  ${CORE_LIBRARIES_TO_LINK}
)

target_include_directories( MpipCore PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Definitions 
set( CORE_DEFINITIONS_TO_ADD ${CORE_DEFINITIONS_TO_ADD} CACHE INTERNAL "")
set( CORE_DEFINITIONS_TO_REMOVE ${CORE_DEFINITIONS_TO_REMOVE} CACHE INTERNAL "")