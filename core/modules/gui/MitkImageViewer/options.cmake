# MODULE_NAME:
#   Every module has a name and builds only one library with the same name
#   This is used by the module's CMakeLists.txt (see template)
# ON_OR_OFF_BY_DEFAULT:
#   The default cmake option value
# NEEDS_MODULES:
#   A list of other modules to turn on if this module needs them
set( MODULE_NAME MitkImageViewer )
set( ON_OR_OFF_BY_DEFAULT OFF )
set( NEEDS_MODULES  )

# CREATE_MODULE is a macro in core/cmake.
# It creates a cmake option and adds the subdirectory if necessary
CREATE_MODULE( ${MODULE_NAME} ${ON_OR_OFF_BY_DEFAULT} ${NEEDS_MODULES} )
