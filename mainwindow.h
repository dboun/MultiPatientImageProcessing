#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>

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

public slots:
    void OnOpenDicom();
    void OnDisplayDicomMetaData();

protected:
  void Load(QString filepath);

private:
  void SetupWidgets();

    Ui::MainWindow *ui;
    mitk::StandaloneDataStorage::Pointer m_DataStorage;
    mitk::Image::Pointer m_DisplayedImage;
    //DicomReader *dicomReader;
    //DicomMetaDataDisplayWidget *dcmdisplayWidget;
};

#endif // MAINWINDOW_H
