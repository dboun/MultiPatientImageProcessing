#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QCloseEvent>
#include <QString>

#include <set> 

namespace Ui {
  class MainWindow;
}

class DataManager;
class SchedulerBase;
class DataViewBase;
class ImageViewerBase;
class AlgorithmModuleBase;
class GuiModuleBase;
class GeodesicTrainingGUI;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void dragEnterEvent(QDragEnterEvent *e) override;
  void dropEvent(QDropEvent *e) override;
  void closeEvent(QCloseEvent *e) override;

public slots:
  // Internal
  void OnOpenSubjects();
  void OnOpenImagesForNewSubject();
  void OnOpenImagesForSelectedSubject();
  void OnCloseAllSubjects();
  void OnTabSelected(int tab);
  void OnToggleFullscreen();
    
private:

  //void EnableRunButton();
  //void DisableRunButton();

  Ui::MainWindow *ui;

  QString       m_AppName           = QString("MLL Semi-Automatic Segmentation");
  QString       m_AppNameShort      = QString("MLL");
  QStringList   m_AcceptedFileTypes = QStringList() << "*.nii.gz" << "*.nii" 
                                                    << "*.nrrd"   << "*.mhd"   << "*.png" 
                                                    << "*.dcm"    << "*.dicom";
  
  QString       m_MostRecentDir     = QString("/home");

  DataManager*     m_DataManager;
  SchedulerBase*   m_Scheduler; 
  ImageViewerBase* m_ImageViewer;
  DataViewBase*    m_DataView;
  GuiModuleBase*   m_GeodesicTrainingGUI;

  bool m_IsSegmentationPanelOpen = false;
};

#endif // ! MAINWINDOW_H
