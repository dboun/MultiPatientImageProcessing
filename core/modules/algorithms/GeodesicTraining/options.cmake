# Give here the name of the library you want to generate
# This is used by the module's CMakeLists.txt as
# project/library name
set( MODULE_NAME ModuleGeodesicTraining )

# Don't change anything below unless your module
# depends on another module.
# In that case, change the other module's option value.
#
# Maybe you also want to add a better option description.

set( OPTION_NAME BUILD_${MODULE_NAME} )
set( DEFINITION_NAME -D${MODULE_NAME} )

option( ${OPTION_NAME} 
  "Build ${MODULE_NAME}" 
  OFF
)

# The if-else statement adds the subproject if necessary
# and also accumulates definitions so they are added by
# master.cmake. That way they are global to the application.
if( ${OPTION_NAME} )
  add_subdirectory( ${CMAKE_CURRENT_LIST_DIR} )
  
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
