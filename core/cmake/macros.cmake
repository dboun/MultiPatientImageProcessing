macro( CREATE_MODULE MODULE_NAME ON_OR_OFF_BY_DEFAULT )
    set ( NEEDS_MODULES ${ARGN} ) # This is not a required argument

    set( OPTION_NAME BUILD_MODULE_${MODULE_NAME} )
    set( DEFINITION_NAME -D${OPTION_NAME} )

    option( ${OPTION_NAME} 
      "Build ${MODULE_NAME}" 
      ${ON_OR_OFF_BY_DEFAULT}
    )

    # The if-else statement adds the subproject if necessary
    # and also accumulates definitions so they are added by
    # master.cmake. That way they are global to the application.
    # Note: CMAKE_CURRENT_LIST_DIR refers to the directory calling
    if( ${OPTION_NAME} )
      message( STATUS "Adding module ${MODULE_NAME}." )
      message( STATUS "Use definition ${DEFINITION_NAME}" )
      
      # Turn on modules that this needs
      foreach( MODULE_TO_TURN_ON ${NEEDS_MODULES} )
        message( STATUS "Trying to turn on " ${MODULE_TO_TURN_ON} )
        set(BUILD_MODULE_${MODULE_TO_TURN_ON} ON 
          CACHE BOOL "Build ${MODULE_TO_TURN_ON}" FORCE
        )
      endforeach()

      add_subdirectory( ${CMAKE_CURRENT_LIST_DIR} )

      target_link_libraries( ${MODULES_LIBRARY}  
        INTERFACE ${MODULE_NAME}
      )
      
      set( CORE_DEFINITIONS_TO_ADD ${CORE_DEFINITIONS_TO_ADD}
        ${DEFINITION_NAME} 
        CACHE INTERNAL ""
      )
    else( ${OPTION_NAME} )
      set( CORE_DEFINITIONS_TO_REMOVE ${CORE_DEFINITIONS_TO_REMOVE}
        ${DEFINITION_NAME} 
        CACHE INTERNAL ""
      )
    endif( ${OPTION_NAME} )

endmacro()
