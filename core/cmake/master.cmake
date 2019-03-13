include ( core/cmake/macros.cmake )

add_subdirectory( core )

add_definitions( ${CORE_DEFINITIONS_TO_ADD} )
remove_definitions( ${CORE_DEFINITIONS_TO_REMOVE} )
