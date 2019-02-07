# Modular applications (add option + add subdirectory + append include dirs for more)

option(BUILD_GEODESIC_TRAINING "Enable building of Geodesic Training" ON)

if (BUILD_GEODESIC_TRAINING)
  add_subdirectory(GeodesicTraining)
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/include/GeodesicTrainingSegmentation" 
       CACHE INTERNAL ""
  )
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/include/GeodesicTrainingSegmentation/cbica_toolkit" 
       CACHE INTERNAL ""
  )
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/SvmSuite/include/SvmSuite" 
       CACHE INTERNAL ""
  )
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/RandomForestSuite/include/RandomForestSuite" 
       CACHE INTERNAL ""
  )
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} 
      "${PROJECT_SOURCE_DIR}/Applications/GeodesicTraining/GeodesicTrainingSegmentation/AdaptiveGeodesicDistance/include/AdaptiveGeodesicDistance" 
       CACHE INTERNAL ""
  )
  set(APP_LIBRARIES ${APP_LIBRARIES}
      GeodesicTrainingSegmentation
      CACHE INTERNAL ""
  )
endif()

