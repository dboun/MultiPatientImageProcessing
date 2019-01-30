#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include "QmitkStdMultiWidget.h"
#include <mitkIOUtil.h>

//#include "DicomMetaDataDisplayWidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

  m_DataStorage = mitk::StandaloneDataStorage::New();

    ui->setupUi(this);

    //dicomReader = new DicomReader();
    //dcmdisplayWidget = new DicomMetaDataDisplayWidget();
    this->SetupWidgets();


    connect(ui->actionOpen_Dicom,SIGNAL(triggered()),this,SLOT(OnOpenDicom()));
    connect(ui->actionDisplay_Metadata,SIGNAL(triggered()),this,SLOT(OnDisplayDicomMetaData()));

}

MainWindow::~MainWindow()
{
    //if(dicomReader)
    //    delete dicomReader;
    //if(dcmdisplayWidget)
    //    delete dcmdisplayWidget;
    delete ui;
}

void MainWindow::OnOpenDicom()
{
  QString filepath = QFileDialog::getOpenFileName(this,tr("Open File"),QDir::currentPath());
  this->Load(filepath);

    //QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
    //                                                QDir::currentPath(),
    //                                                QFileDialog::ShowDirsOnly
    //                                                | QFileDialog::DontResolveSymlinks);

    //dicomReader->SetDirectoryPath(dir.toStdString());
    //dicomReader->ReadMetaData();
    ////dicomReader->PrintMetaData();

    ////test get tag value;
    //std::string val,label;
    //std::string tag = "0008|103e";//"0010|0010";
    //bool found = dicomReader->GetTagValue(tag,label, val);
    //if(found)
    //    std::cout << " tag: " << tag << " description: " << label << " value: " << val << std::endl;
    //else
    //    std::cout << " tag not found " << std::endl;
}

void MainWindow::OnDisplayDicomMetaData()
{
    ////dicomReader->PrintMetaData();
    //dcmdisplayWidget->UpdateDicomData(dicomReader->GetMetaDataMap());
    //dcmdisplayWidget->show();
}

void MainWindow::Load(QString filepath)
{
  // Load datanode (eg. many image formats, surface formats, etc.)
  mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(filepath.toStdString(), *m_DataStorage);

  if (dataNodes->empty())
  {
    fprintf(stderr, "Could not open file %s \n\n", filepath.toStdString());
    exit(2);
  }

  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
  if ((m_FirstImage.IsNull()) && (image.IsNotNull()))
    m_FirstImage = image;
}

void MainWindow::SetupWidgets()
{
  QmitkStdMultiWidget *multiWidget = new QmitkStdMultiWidget();
  ui->centralWidget->layout()->addWidget(multiWidget);

  // Tell the multiWidget which DataStorage to render
  multiWidget->SetDataStorage(m_DataStorage);

  // Initialize views as axial, sagittal, coronar (from
  // top-left to bottom)
  auto geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  // Initialize bottom-right view as 3D view
  multiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Enable standard handler for levelwindow-slider
  multiWidget->EnableStandardLevelWindow();

  // Add the displayed views to the DataStorage to see their positions in 2D and 3D
  multiWidget->AddDisplayPlaneSubTree();
  multiWidget->AddPlanesToDataStorage();
  multiWidget->SetWidgetPlanesVisibility(true);
}
