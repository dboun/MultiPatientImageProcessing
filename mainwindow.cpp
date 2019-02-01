#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
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

	setAcceptDrops(true); // For drag and drop
 	ui->patientTree->setHeaderLabel("Select subjects");

    connect(ui->actionOpen_Dicom,SIGNAL(triggered()),this,SLOT(OnOpenDicom()));
    connect(ui->actionDisplay_Metadata,SIGNAL(triggered()),this,SLOT(OnDisplayDicomMetaData()));
	connect(ui->actionOpen_single_subject, SIGNAL(triggered()), this, SLOT(OnOpenSingleSubject()));
	connect(ui->pushButtonConfigure, SIGNAL(released()), this, SLOT(handleConfigButton()));
}

MainWindow::~MainWindow()
{
    //if(dicomReader)
    //    delete dicomReader;
    //if(dcmdisplayWidget)
    //    delete dcmdisplayWidget;
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *e)
{
	foreach(const QUrl &url, e->mimeData()->urls()) {
		QString fileName = url.toLocalFile();
		qDebug(("Dropped file:" + fileName).toStdString().c_str());
	}

	// Not implemented yet message
	QMessageBox::information(
		this,
		tr("MPIP"),
		tr("Drag and drop is not implemented yet.")
	);
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

void MainWindow::OnOpenSingleSubject()
{
	/*QString filename = QFileDialog::getOpenFileName(
		this,
		"Open Document",
		QDir::currentPath(),
		"All files (*.*) ;; Document files (*.doc *.rtf);; PNG files (*.png)"
	);*/

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks
	);

	if (!dir.isEmpty() && LoadSingleSubject(dir))
	{
		SwitchSubjectAndImage(m_Subjects.size() - 1);
	}
}

void MainWindow::handleConfigButton()
{
	QStringList apps;

#ifdef BUILD_GEODESIC_TRAINING
	apps << "Geodesic Training Segmentation";
#endif // BUILD_GEODESIC_TRAINING


}

void MainWindow::Load(QString filepath)
{
  // Load datanode (eg. many image formats, surface formats, etc.)
  if (filepath.toStdString() != "") 
  {
	  mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(filepath.toStdString(), *m_DataStorage);
  
	if (dataNodes->empty())
	{
		fprintf(stderr, "BCould not open file %s \n\n", filepath.toStdString().c_str());
		//exit(2);
	}
	else {
		mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
		/*if ((m_FirstImage.IsNull()) && (image.IsNotNull()))
		m_FirstImage = image;*/
	}
  }

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

bool MainWindow::LoadSingleSubject(QString directoryPath)
{
	std::lock_guard<std::mutex> lg(m_SubjectsMutex); // Lock mutex (gets unlocked on function finishing)

	size_t pos = m_Subjects.size(); // The position to add
	m_Subjects.push_back( QStringList() );

	LoadAllFilesRecursive(directoryPath, pos);

	//qDebug(files.at(0).toStdString().c_str());

	if (m_Subjects[pos].isEmpty()) {
		m_Subjects.pop_back();
		return false;
	}
	
	//qDebug(std::to_string(m_Subjects[pos].size()).c_str());
	
	// Update tree widget
	QTreeWidgetItem *patientToAdd = new QTreeWidgetItem(ui->patientTree);
	patientToAdd->setText(0,
		QString::fromStdString(
			directoryPath.toStdString().substr(
				directoryPath.toStdString().find_last_of("/\\")+1
			)
		)
	);
	patientToAdd->setCheckState(0, Qt::Checked);

	foreach(QString file, m_Subjects[pos]) {
		QTreeWidgetItem *styleItem = new QTreeWidgetItem(patientToAdd);
		styleItem->setText(0,
			QString::fromStdString(
				file.toStdString().substr(
					file.toStdString().find_last_of("/\\") + 1
				)
			)
		);
		styleItem->setCheckState(0, Qt::Checked);
		//styleItem->setData(0, Qt::UserRole, QVariant(database.weight(family, style)));
		//styleItem->setData(0, Qt::UserRole + 1, QVariant(database.italic(family, style)));
	}

	return true;
}

void MainWindow::LoadAllFilesRecursive(QString directoryPath, size_t pos)
{
	QDir dir = QDir(directoryPath);

	QStringList files = dir.entryList(m_AcceptedFileTypes,
		QDir::Files | QDir::NoSymLinks);

	QStringList subdirectories = dir.entryList(m_AcceptedFileTypes,
		QDir::Dirs | QDir::NoSymLinks);

	//qDebug(files.at(0).toStdString().c_str());

	// Add all files to the patient list
	for (const auto& file : files)
	{
		m_Subjects[pos] << directoryPath + QString("/") + file;
		//qDebug(m_Subjects[pos].at(m_Subjects[pos].size()-1).toStdString().c_str());
	}

	// Do the same for subdirectories
	for (const auto& subdir : subdirectories)
	{
		LoadAllFilesRecursive(subdir, pos);
	}
}

void MainWindow::SwitchSubjectAndImage(size_t subjectPos, size_t imagePos)
{
	m_CurrentSubject = subjectPos;
	Load(m_Subjects[subjectPos].at(imagePos));
}
