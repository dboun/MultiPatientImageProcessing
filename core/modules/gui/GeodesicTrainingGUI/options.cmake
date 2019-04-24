# MODULE_NAME:
#   Every module has a name and builds only one library with the same name
#   This is used by the module's CMakeLists.txt (see template)
# ON_OR_OFF_BY_DEFAULT:
#   The default cmake option value
# NEEDS_MODULES:
#   A list of other modules to turn on if this module needs them
set( MODULE_NAME GeodesicTrainingGUI )
set( ON_OR_OFF_BY_DEFAULT OFF )
set( NEEDS_MODULES GeodesicTraining MitkGeneral MitkSegmentationTool )