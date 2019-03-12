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
SUBDIRLIST( ALGORITHM_MODULES_PATHS 
  ${PROJECT_SOURCE_DIR}/algorithms 
)
SUBDIRLIST( GUI_MODULES_PATHS 
  ${PROJECT_SOURCE_DIR}/gui 
)
SUBDIRLIST( OTHER_MODULES_PATHS 
  ${PROJECT_SOURCE_DIR}/other 
)

# Unify all the modules in one list
foreach( ALGORITHM_MODULE_PATH ${ALGORITHM_MODULES_PATHS} )
  set( MODULES_PATHS ${MODULES_PATHS}
    algorithms/${ALGORITHM_MODULE_PATH}
  )
endforeach()
foreach( GUI_MODULE_PATH ${GUI_MODULES_PATHS} )
  set( MODULES_PATHS ${MODULES_PATHS}
    gui/${GUI_MODULE_PATH}
  )
endforeach()
foreach( OTHER_MODULE_PATH ${OTHER_MODULES_PATHS} )
  set( MODULES_PATHS ${MODULES_PATHS}
    other/${OTHER_MODULE_PATH}
  )
endforeach()

# Create a library without sources
add_library( MpipModules INTERFACE )

# add all enabled subproject module libraries to the library
foreach( MODULE_PATH ${MODULES_PATHS} )
  include( ${MODULE_PATH}/options.cmake )
  target_link_libraries( MpipModules INTERFACE 
    ${MODULE_NAME}
  )
endforeach()
