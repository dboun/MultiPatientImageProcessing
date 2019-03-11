project( MpipModules )

# Macro to find all subdirectories
# (curdir is the parent folder of the subdirectories)
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Find all modules
SUBDIRLIST( ALGORITHM_MODULES_PATHS algorithms )
SUBDIRLIST( GUI_MODULES_PATHS gui )

# Unify all the modules in one list
set( MODULES_PATHS 
  ${ALGORITHM_MODULES_PATHS} 
  ${GUI_MODULES_PATHS} 
)

# Create a library without sources
add_library( MpipModules INTERFACE )

# add all enabled subproject module libraries to the library
foreach( MODULE_PATH ${MODULES_PATHS}
  include( ${MODULE_PATH}/options.cmake )
  target_link_libraries( MpipModules
    ${MODULE_NAME}
  )
)