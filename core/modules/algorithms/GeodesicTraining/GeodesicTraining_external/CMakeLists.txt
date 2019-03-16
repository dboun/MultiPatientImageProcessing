cmake_minimum_required(VERSION 3.0)
SET( PROJECT_NAME GeodesicTrainingSegmentation)
project (${PROJECT_NAME} VERSION 1.0 DESCRIPTION "Geodesic Training Segmentation for medical images")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES)

# For ITK
FIND_PACKAGE( ITK REQUIRED )
SET(ITK_NO_IO_FACTORY_REGISTER_MANAGER "OFF")

# For OpenCV
FIND_PACKAGE( OpenCV REQUIRED )

# Setting paths for headers and sources
SET( Project_Include_Dir "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}" )
SET( Project_Src_Dir "${PROJECT_SOURCE_DIR}/src" )

# Setting the sources
FILE(GLOB_RECURSE GEODESIC_TRAINING_SEGMENTATION_Sources "${Project_Src_Dir}/*.*")
FILE(GLOB_RECURSE CBICA_TOOLKIT_Sources "${Project_Include_Dir}/cbica_toolkit/*.cpp")

# Adding the subprojects
add_subdirectory(AdaptiveGeodesicDistance)
add_subdirectory(SvmSuite)
add_subdirectory(RandomForestSuite)

# For ITK
FIND_PACKAGE( ITK REQUIRED )
SET(ITK_NO_IO_FACTORY_REGISTER_MANAGER "OFF")

# Creating the library
add_library(${PROJECT_NAME} STATIC 
	${GEODESIC_TRAINING_SEGMENTATION_Sources}
	${CBICA_TOOLKIT_Sources}
)

# The paths to the APIs of the subprojects
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${Project_Include_Dir}
	${Project_Include_Dir}/cbica_toolkit
	${ITK_INCLUDE_DIRS}
)

# Linking the dependencies of the library
TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    AdaptiveGeodesicDistance
    SvmSuite
	RandomForestSuite
    ${ITK_LIBRARIES}
	${OpenCV_LIBRARIES}
)
