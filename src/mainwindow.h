#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QString>

#include <set>

#include "DataManager.h"
#include "DefaultScheduler.h"
#include "ImageViewerBase.h"
#include "DataViewBase.h"

#ifdef BUILD_MODULE_MitkDrawingTool
#include "MitkDrawingTool.h"
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
  
  // Slots for Scheduler
  void OnSchedulerJobFinished(AlgorithmModuleBase* algorithmModuleBase);

  // Slots for AlgorithmModuleBase
  void OnAlgorithmFinished(AlgorithmModuleBase* algorithmModuleBase);
  void OnAlgorithmFinishedWithError(AlgorithmModuleBase* algorithmModuleBase, 
    QString errorMessage);
  
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
  QString       m_AppNameShort      = QString("MLL");
  QStringList   m_AcceptedFileTypes = QStringList() << "*.nii.gz" << "*.nrrd";
  
  QString       m_MostRecentDir     = QString("/home");

  DataManager*     m_DataManager;
  SchedulerBase*   m_Scheduler; 
  ImageViewerBase* m_ImageViewer;
  DataViewBase*    m_DataView;
  
#ifdef BUILD_MODULE_MitkDrawingTool
  MitkDrawingTool *m_MitkDrawingTool;
#endif

  bool m_IsSegmentationPanelOpen = false;
  std::set<long> m_SubjectsThatAreRunning;

};

#endif // ! MAINWINDOW_H
