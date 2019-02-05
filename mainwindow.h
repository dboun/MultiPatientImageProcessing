#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>

//#include "GeodesicTrainingSegmentation.h"
#include "GeodesicTrainingSegmentation.h"

#include <mutex>
#include <vector>

//class DicomReader;
//class DicomMetaDataDisplayWidget;

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
    void OnDisplayDicomMetaData();
	void OnOpenSingleSubject();
	void handleConfigButton();
	void OnTreeWidgetClicked(QTreeWidgetItem *item, int column);

protected:
  void Load(QString filepath);

private:
  void SetupWidgets();

  /** Loads all the images of a single patient */
  bool LoadSingleSubject(QString directoryPath);

  /** Loads all the images from a directory, and calls itself for subdirectories */
  void LoadAllFilesRecursive(QString directoryPath, size_t pos);

  void SwitchSubjectAndImage(size_t subjectPos, size_t imagePos = 0);

    Ui::MainWindow *ui;
    mitk::StandaloneDataStorage::Pointer m_DataStorage; // Used for MITK image displaying (use load to show image)
    //mitk::Image::Pointer m_DisplayedImage;
    
	//DicomReader *dicomReader;
    //DicomMetaDataDisplayWidget *dcmdisplayWidget;

	std::vector< QStringList > m_Subjects;
	size_t m_CurrentSubject; // index of m_Subjects
	std::mutex m_SubjectsMutex; // Used so that no parallel additions/deletions happen simultaneously

	QStringList m_AcceptedFileTypes = QStringList() << "*.nii.gz" << "*.dcm" << "*.dicom";
};

#endif // MAINWINDOW_H
