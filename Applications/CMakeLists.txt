# Modular applications

set( APP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/Applications" )
unset( APP_LIBRARIES )
unset( APP_HEADERS )
unset( APP_SOURCES )

# APP_INCLUDE_DIRS: contains all the include directories from within the modular application
# APP_LIBRARIES: contains all the libraries created by the application's own CMakeLists.txt
# APP_HEADERS: all files that extend ApplicationBase
# APP_SOURCES: all implemententions of classes that extend ApplicationBase

include( ApplicationGeodesicTrainingSegmentation.cmake )