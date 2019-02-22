#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QString>

#include "Scheduler.h"
#include "DataManager.h"
#include "ImageViewerBase.h"
#include "DataViewBase.h"
#include "ui_mainwindow.h"

#ifdef BUILD_VIEWER
#include "MitkViewer.h"
#endif
#include "DataTreeView.h"

class MPIPQmitkSegmentationPanel;
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
  void RunPressed();
  void SchedulerResultReady(long uid);
  void SelectedSubjectChangedHandler(long uid);
  void OnSegmentationButtonClicked();

signals:
  void EnableSegmentation();
  void DisableSegmentation();

private:

  //void EnableRunButton();
  //void DisableRunButton();

  Ui::MainWindow *ui;

  QStringList   m_AcceptedFileTypes = QStringList() << "*.nii.gz" << "*.nrrd";
  QString       m_MostRecentDir     = QString("/home");

  Scheduler        m_Scheduler;
  
  DataManager*     m_DataManager;
  ImageViewerBase* m_ImageViewer;
  DataViewBase*    m_DataView;
  MPIPQmitkSegmentationPanel *m_SegmentationPanel;

  long m_CurrentSubjectID = -1;
  bool m_IsSegmentationPanelOpen = false;

};

#endif // ! MAINWINDOW_H
