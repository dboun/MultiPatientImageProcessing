#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QString>

#include "DataManager.h"
#include "SchedulerBase.h"
#include "ImageViewerBase.h"
#include "DataViewBase.h"

#ifdef BUILD_MITK
#include "MitkViewer.h"
class MPIPQmitkSegmentationPanel;
#endif

#ifdef BUILD_GEODESIC_TRAINING
#include "Scheduler.h"
#endif

#include "DataTreeView.h"
#include "ui_mainwindow.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void dragEnterEvent(QDragEnterEvent *e) override;
  void dropEvent(QDropEvent *e) override;

public slots:
  void OnOpenSingleSubject();
  //void HandleConfigButton(); 
  void OnRunPressed();
  //void OnSchedulerResultReady(long uid);
  void OnSchedulerJobQueued(long uid);
  
#ifdef BUILD_MITK
  void OnSegmentationButtonClicked();
#endif

  void SelectedSubjectChangedHandler(long uid);

signals:

#ifdef BUILD_MITK
  void EnableSegmentation(); // Segmentation here means the drawing tools
  void DisableSegmentation();
#endif

private:

  //void EnableRunButton();
  //void DisableRunButton();

  Ui::MainWindow *ui;

  QString       m_AppName           = QString("MLL Semi-Automatic Segmentation");
  QStringList   m_AcceptedFileTypes = QStringList() << "*.nii.gz" << "*.nrrd";
  QString       m_MostRecentDir     = QString("/home");

  DataManager*     m_DataManager;
  SchedulerBase*   m_Scheduler; 
  ImageViewerBase* m_ImageViewer;
  DataViewBase*    m_DataView;
  
#ifdef BUILD_MITK
  MPIPQmitkSegmentationPanel *m_SegmentationPanel;
#endif

  long m_CurrentSubjectID = -1;
  bool m_IsSegmentationPanelOpen = false;

};

#endif // ! MAINWINDOW_H
