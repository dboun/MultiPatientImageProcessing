# Modular applications (add option + add subdirectory + append include dirs for more)
set(APP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/Applications")

option(BUILD_GEODESIC_TRAINING "Enable building of Geodesic Training" ON)

if (BUILD_GEODESIC_TRAINING)
  message(STATUS "Building with Geodesic Training")
  add_definitions(-DBUILD_GEODESIC_TRAINING)
  add_subdirectory(GeodesicTraining)
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/include/GeodesicTrainingSegmentation" 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/include/GeodesicTrainingSegmentation/cbica_toolkit" 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/SvmSuite/include/SvmSuite" 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/RandomForestSuite/include/RandomForestSuite" 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/AdaptiveGeodesicDistance/include/AdaptiveGeodesicDistance" 
       CACHE INTERNAL ""
  )
  set(APP_LIBRARIES ${APP_LIBRARIES}
      GeodesicTrainingSegmentation
      CACHE INTERNAL ""
  )
  set(APP_HEADERS ${APP_HEADERS}
      "${PROJECT_SOURCE_DIR}/Applications/ApplicationGeodesicTrainingSegmentation.h"
      CACHE INTERNAL ""
  )
  set(APP_SOURCES ${APP_SOURCES}
      "${PROJECT_SOURCE_DIR}/Applications/ApplicationGeodesicTrainingSegmentation.cpp"
      CACHE INTERNAL ""
  )
endif()