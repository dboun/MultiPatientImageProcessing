unset( CORE_DEFINITIONS_TO_ADD CACHE )
unset( CORE_DEFINITIONS_TO_REMOVE CACHE )
include ( core/cmake/macros.cmake )

# For OpenMP
FIND_PACKAGE(OpenMP REQUIRED)
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )

add_subdirectory( core )

message( STATUS "Available definitions are: ${CORE_DEFINITIONS_TO_ADD}" )
message( STATUS "Removed definitions are:   ${CORE_DEFINITIONS_TO_REMOVE}" )
add_definitions( ${CORE_DEFINITIONS_TO_ADD} )
remove_definitions( ${CORE_DEFINITIONS_TO_REMOVE} )
