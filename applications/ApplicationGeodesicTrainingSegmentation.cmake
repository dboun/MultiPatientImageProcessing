option(BUILD_GEODESIC_TRAINING "Enable building of Geodesic Training" ON)

if (BUILD_GEODESIC_TRAINING)
  message(STATUS "Building with Geodesic Training")
  set( APPLICATION_DEFINITIONS_TO_ADD ${APPLICATION_DEFINITIONS_TO_ADD} -DBUILD_GEODESIC_TRAINING CACHE INTERNAL "")
  add_subdirectory(GeodesicTraining)
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/applications/GeodesicTraining/GeodesicTrainingSegmentation/include/GeodesicTrainingSegmentation" 
      "${PROJECT_SOURCE_DIR}/applications/GeodesicTraining/GeodesicTrainingSegmentation/include/GeodesicTrainingSegmentation/cbica_toolkit" 
      "${PROJECT_SOURCE_DIR}/applications/GeodesicTraining/GeodesicTrainingSegmentation/SvmSuite/include/SvmSuite" 
      "${PROJECT_SOURCE_DIR}/applications/GeodesicTraining/GeodesicTrainingSegmentation/RandomForestSuite/include/RandomForestSuite" 
      "${PROJECT_SOURCE_DIR}/applications/GeodesicTraining/GeodesicTrainingSegmentation/AdaptiveGeodesicDistance/include/AdaptiveGeodesicDistance" 
      CACHE INTERNAL ""
  )
  set(APP_LIBRARIES ${APP_LIBRARIES}
      GeodesicTrainingSegmentation
      ${ITK_LIBRARIES}
      CACHE INTERNAL ""
  )
  set(APP_HEADERS ${APP_HEADERS}
      "${PROJECT_SOURCE_DIR}/applications/ApplicationGeodesicTrainingSegmentation.h"
      CACHE INTERNAL ""
  )
  set(APP_SOURCES ${APP_SOURCES}
      "${PROJECT_SOURCE_DIR}/applications/ApplicationGeodesicTrainingSegmentation.cpp"
      CACHE INTERNAL ""
  )
else()
  set( APPLICATION_DEFINITIONS_TO_REMOVE ${APPLICATION_DEFINITIONS_TO_REMOVE} -DBUILD_GEODESIC_TRAINING CACHE INTERNAL "")
endif()