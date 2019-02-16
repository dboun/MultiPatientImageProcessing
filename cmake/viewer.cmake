option(BUILD_VIEWER "Build image viewer" ON)

if( BUILD_VIEWER )
  
  add_definitions(-DBUILD_VIEWER)

  find_package( MITK REQUIRED )
  #find_package{ VTK REQUIRED }
  
  set( VIEWER_LIBRARIES_TO_LINK 
      MitkCore MitkMultilabel MitkQtWidgets MitkQtWidgetsExt
  )

  set( CPP_FILES ${CPP_FILES}
    #VtkViewer.cpp
    MpipMitkViewer.cpp
  )

  set( MOC_H_FILES ${MOC_H_FILES}
    #VtkViewer.h
    MpipMitkViewer.h
  )

  set( UI_FILES ${UI_FILES}
    #VtkViewer.ui
  )

  set( H_FILES ${H_FILES}
    #VtkViewer.h
    MpipMitkViewer.h
  )

else()
  remove_definitions(-DBUILD_VIEWER)
endif()