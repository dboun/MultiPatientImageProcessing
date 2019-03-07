option(BUILD_MITK "Build MITK image viewer and drawing tool" ON)

if( BUILD_MITK )
  
  set( CORE_DEFINITIONS_TO_ADD ${CORE_DEFINITIONS}
  	-DBUILD_MITK
  	CACHE INTERNAL ""
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
    CACHE INTERNAL ""
  )
endif()
