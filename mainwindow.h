#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include "vtkSmartPointer.h"
#include "vtkResliceImageViewer.h"
#include "vtkImagePlaneWidget.h"
//#include <mitkStandaloneDataStorage.h>
//#include <mitkImage.h>

#include "Scheduler.h"
//#include "GeodesicTrainingSegmentation.h"

#include <mutex>
#include <vector>

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
  void OnOpenDicom();
  void OnOpenSingleSubject();
  void handleConfigButton();
  void OnTreeWidgetClicked(QTreeWidgetItem *item, int column);
  void ShowTreeContextMenu(const QPoint& pos);
  void TreeContextRemoveItem();
  void TreeContextSetItemAsMask();
  void RunPressed();
  void SchedulerResultReady(long uid);
  void UpdateProgress(long uid, int progress);

protected:
  void Load(QString filepath);

private:
  //void SetupWidgets();
  void ConstructViews(vtkImageData *image);
  void WriteVTKImage(vtkImageData*, std::string filename);

  /** Loads all the images of a single patient */
  bool LoadSingleSubject(QString directoryPath);

  /** Loads all the images from a directory, and calls itself for subdirectories */
  void LoadAllFilesRecursive(QString directoryPath, QStringList& allFiles);

  void SwitchSubjectAndImage(size_t subjectPos, size_t imagePos = 0);

  Ui::MainWindow *ui;
  //mitk::StandaloneDataStorage::Pointer m_DataStorage; // Used for MITK image displaying (use load to show image)

  std::mutex m_TreeEditMutex; // Used so that no parallel additions/deletions happen simultaneously

  QStringList m_AcceptedFileTypes = QStringList() << "*.nii.gz" << "*.dcm" << "*.dicom";

  long uidNextToGive = 0;
  std::map<long, QTreeWidgetItem*> subjectByUid;

  vtkSmartPointer< vtkResliceImageViewer > riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > planeWidget[3];

  Scheduler m_Scheduler;
  //QString m_dir;
};

#endif // MAINWINDOW_H
