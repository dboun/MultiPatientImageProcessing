set( CPP_FILES
  Main.cpp
  MainWindow.cpp
  #VtkViewer.cpp
  MpipMitkViewer.cpp
  Scheduler.cpp
  ApplicationBase.cpp
  ${APP_SOURCES}
)

set( MOC_H_FILES
  MainWindow.h
  #VtkViewer.h
  MpipMitkViewer.h
  Scheduler.h
  ApplicationBase.h
  ${APP_HEADERS}
)

set( UI_FILES
  MainWindow.ui
  #VtkViewer.ui
)

set( H_FILES
  MainWindow.h
  #VtkViewer.h
  MpipMitkViewer.h
  Scheduler.h
  ApplicationBase.h
  ${APP_HEADERS}
)