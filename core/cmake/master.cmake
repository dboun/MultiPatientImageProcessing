cmake_minimum_required(VERSION 3.1.0)

# Base contains the data manager and interfaces for modules
add_subdirectory( ${CMAKE_CURRENT_LIST_DIR}/../base )

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

SUBDIRLIST( APPLICATION_MODULES 
  ${CMAKE_CURRENT_LIST_DIR}/../modules/applications 
)

SUBDIRLIST( GUI_MODULES
  ${CMAKE_CURRENT_LIST_DIR}/../modules/gui
)

set( MODULES 
  ${APPLICATION_MODULES}
  ${GUI_MODULES}
)

# Options
option(BUILD_MITK_VIEWER "Build MITK image viewer" ON)
option(BUILD_MITK_DRAWING_TOOL "Build MITK drawing tool (needs viewer)" ON)
option(BUILD_GEODESIC_TRAINING "Build GeodesicTraining Segmentation" OFF)

if( BUILD_MITK_DRAWING_TOOL )
  add_definitions( -DBUILD_MITK -DBUILD_MITK_DRAWING_TOOL)  
  set(BUILD_MITK_VIEWER ON CACHE BOOL "Build MITK image viewer")
endif( BUILD_MITK_DRAWING_TOOL )

if( BUILD_MITK_VIEWER )
  add_definitions( -DBUILD_MITK -DBUILD_MITK_VIEWER)  
endif( BUILD_MITK_VIEWER )

if( BUILD_GEODESIC_TRAINING )
  add_definitions( -DBUILD_GEODESIC_TRAINING)
endif( BUILD_GEODESIC_TRAINING )

if( BUILD_MITK )
  
  set( CORE_DEFINITIONS_TO_ADD ${CORE_DEFINITIONS_TO_ADD}
  	-DBUILD_MITK
  )

  find_package( MITK REQUIRED )
  
  set( CORE_LIBRARIES_TO_LINK ${CORE_LIBRARIES_TO_LINK}  
      MitkCore 
      MitkMultilabel 
      MitkQtWidgets 
      MitkQtWidgetsExt
      MitkSegmentation
      MitkSegmentationUI
  )

  set( CPP_FILES ${CPP_FILES}
    MitkViewer.cpp
    MPIPQmitkSegmentationPanel.cpp
  )

  set( MOC_H_FILES ${MOC_H_FILES}
    MitkViewer.h
    MPIPQmitkSegmentationPanel.h
  )

  set( UI_FILES ${UI_FILES}
    MPIPQmitkSegmentationPanel.ui
  )

  set( H_FILES ${H_FILES}
    MitkViewer.h
    MPIPQmitkSegmentationPanel.h
  )

else()
  set( CORE_DEFINITIONS_TO_REMOVE ${CORE_DEFINITIONS_TO_REMOVE}
    -DBUILD_MITK
  )
endif()
