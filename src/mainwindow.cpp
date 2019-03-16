#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

#include <vector>

#ifdef BUILD_MODULE_GeodesicTraining
#include "GeodesicTrainingModule.h"
#endif

#ifdef BUILD_MODULE_MitkImageViewer
#include "MitkImageViewer.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
  // Initialize DataManager
  m_DataManager = new DataManager(this);
  m_DataManager->SetAcceptedFileTypes(m_AcceptedFileTypes);
  m_DataManager->SetAppNameShort(m_AppNameShort);

  // Initialize Scheduler
  m_Scheduler = new DefaultScheduler(this);
  
  // Initialize UI
  ui->setupUi(this);
  this->setWindowTitle( m_AppName );

  // Initialize DataView
  m_DataView = new DataTreeView(ui->dataViewContainer);
  m_DataView->AcceptOnlyNrrdMaskAndSegmentations(true);
  m_DataView->setMinimumWidth(350);
  QHBoxLayout *layoutDataViewer = new QHBoxLayout(ui->dataViewContainer);
  layoutDataViewer->addWidget(m_DataView);
  m_DataView->SetDataManager(m_DataManager);
  m_DataView->SetAppName(m_AppName);
  m_DataView->SetAppNameShort(m_AppNameShort);

  // Initialize ImageViewer
#ifdef BUILD_MODULE_MitkImageViewer
  m_ImageViewer = new MitkImageViewer(ui->viewerContainer);
  m_ImageViewer->setMinimumWidth(600);
  m_ImageViewer->setMinimumHeight(500);
#else
  qDebug() << "Using abstract image viewer";
  m_ImageViewer = new ImageViewerBase(ui->viewerContainer);
#endif

  ui->viewerContainer->layout()->addWidget(m_ImageViewer);
  m_ImageViewer->SetDataManager(m_DataManager);
  m_ImageViewer->SetDataView(m_DataView);
  m_ImageViewer->SetOpacitySlider(ui->opacitySlider);
  m_ImageViewer->SetAppName(m_AppName);
  m_ImageViewer->SetAppNameShort(m_AppNameShort);

  // Initialize MitkDrawingTool
#ifdef BUILD_MODULE_MitkDrawingTool
  m_MitkDrawingTool = new MitkDrawingTool(
    qobject_cast<MitkImageViewer*>(m_ImageViewer)->GetDataStorage(), this
  );

  m_MitkDrawingTool->SetMitkImageViewer(
    qobject_cast<MitkImageViewer*>(m_ImageViewer)
  );

  m_MitkDrawingTool->SetDataManager(m_DataManager);
  m_MitkDrawingTool->SetAppName(m_AppName);
  m_MitkDrawingTool->SetAppNameShort(m_AppNameShort);
  this->ui->rightPanel->layout()->addWidget(this->m_MitkDrawingTool);
  //this->m_SegmentationPanel->hide();
#endif

  // Disable unused buttons
  ui->actionAdd_image_for_selected_subject->setVisible(false);
  ui->actionAdd_image_for_new_subject->setVisible(false);
  ui->actionAdd_multiple_subjects->setVisible(false);
  ui->pushButtonConfigure->setVisible(false);

  // Turn on drag and drop
  setAcceptDrops(true); 

  // Shadow effect
  QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
  effect->setBlurRadius(2);
  effect->setXOffset(1);
  effect->setYOffset(1);
  effect->setColor(Qt::black);
  ui->pushButtonRun->setGraphicsEffect(effect);
  ui->dataViewContainer->setGraphicsEffect(effect);

  // Signals and Slots
  connect(ui->pushButtonRun, SIGNAL(released()), 
    this, SLOT(OnRunPressed())
  );
  connect(m_Scheduler, SIGNAL(JobFinished(AlgorithmModuleBase*)), 
    this, SLOT(OnSchedulerJobFinished(AlgorithmModuleBase*))
  );
  connect(ui->actionOpen_single_subject, SIGNAL(triggered()), 
    this, SLOT(OnOpenSingleSubject())
  );
  //connect(ui->pushButtonConfigure, SIGNAL(released()), 
  //  this, SLOT(HandleConfigButton())
  //);
  connect(m_DataView, SIGNAL(SelectedSubjectChanged(long)),
    this, SLOT(SelectedSubjectChangedHandler(long))
  );

#ifdef BUILD_MODULE_MitkDrawingTool
  //connect(ui->SegmentationBtn, SIGNAL(clicked()), this, SLOT(OnSegmentationButtonClicked()));
  //connect(this, SIGNAL(EnableSegmentation()), m_SegmentationPanel, SLOT(OnEnableSegmentation()));
  //connect(this, SIGNAL(DisableSegmentation()), m_SegmentationPanel, SLOT(OnDisableSegmentation()));
  //connect(m_ImageViewer, SIGNAL(DisplayedDataName(long)), m_SegmentationPanel, SLOT(SetDisplayDataName(long)));
#endif
}

MainWindow::~MainWindow()
{
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
    qDebug() << "Dropped file:" + fileName;

    m_DataManager->AddSubjectAndDataByDirectoryPath(fileName);
  }

  //// Not implemented yet message
  //QMessageBox::information(
  //  this,
  //  tr("MPIP"),
  //  tr("Drag and drop is not fully implemented yet.")
  //);
}

void MainWindow::OnOpenSingleSubject()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
    m_MostRecentDir,
    QFileDialog::ShowDirsOnly |
    QFileDialog::DontResolveSymlinks
  );

  m_MostRecentDir = dir;

  if (!dir.isEmpty())
  {
    // DataManager will notify everything that there was a change
    m_DataManager->AddSubjectAndDataByDirectoryPath(dir);
  }
}

void MainWindow::OnRunPressed()
{
	long uid = m_DataView->GetCurrentSubjectID(); // For convenience

	if (uid == -1)
	{
		QMessageBox::information(
			this,
			tr("No subject selected"),
			tr("Please select a subject.")
		);
		return;
	}

	qDebug() << QString("(Run) uid:  ") << QString::number(uid);

	// std::vector<long> iidsOfSubject = m_DataManager->GetAllDataIdsOfSubject(uid);

	// int numberOfImages = 0, numberOfMasks = 0;

	// for (const long& iid : iidsOfSubject)
	// {
	// 	QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid); 

	// 	if (dataSpecialRole == "Mask") {
	// 		numberOfMasks++;
	// 	}
	// 	else if (dataSpecialRole != "Segmentation")
	// 	{
	// 		numberOfImages++;
	// 	}
	// }

	// if (numberOfMasks == 0)
	// {
	// 	QMessageBox::information(
	// 	  this,
	// 	  tr("No mask drawn"),
	// 	  tr("Please create a mask and run again.")
	// 	);
	// 	return;
	// }
	// if (numberOfMasks > 1)
	// {
	// 	QMessageBox::information(
	// 		this,
	// 		tr("Multiple masks"),
	// 		tr("Please remove all but one masks (Right click & Remove won't delete an image from the disk).")
	// 	);
	// 	return;
	// }
	// if (numberOfImages == 0)
	// {
	// 	QMessageBox::information(
	// 		this,
	// 		tr("No images"),
	// 		tr("Please load some images.")
	// 	);
	// 	return;
	// }

#ifdef BUILD_MODULE_MitkImageViewer
  auto iids = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
    uid, "Mask"
  );

  long maskNrrd = -1;

  for (const long& iid : iids)
  {
    if (m_DataManager->GetDataPath(iid).endsWith(".nrrd", Qt::CaseSensitive))
    {
      maskNrrd = iid;
      break;
    }
  }

  if (maskNrrd != -1)
  {
    qobject_cast<MitkImageViewer*>(m_ImageViewer)->SaveImageToFile(maskNrrd);
  }
#endif

#ifdef BUILD_MODULE_GeodesicTraining
	AlgorithmModuleBase* algorithm = new GeodesicTrainingModule();
#else
  AlgorithmModuleBase* algorithm = new AlgorithmModuleBase();
#endif

  algorithm->SetDataManager(m_DataManager);
  algorithm->SetUid(uid);
  algorithm->SetAppName(m_AppName);
  algorithm->SetAppNameShort(m_AppNameShort);

  connect(algorithm, SIGNAL(ProgressUpdateUI(long, QString, int)), 
    m_DataView, SLOT(UpdateProgressHandler(long, QString, int))
  );
  connect(algorithm, SIGNAL(AlgorithmFinished(AlgorithmModuleBase*)),
    this, SLOT(OnAlgorithmFinished(AlgorithmModuleBase*))
  );
  connect(algorithm, SIGNAL(AlgorithmFinishedWithError(AlgorithmModuleBase*, QString)),
    this, SLOT(OnAlgorithmFinishedWithError(AlgorithmModuleBase*, QString))
  );

  m_Scheduler->QueueAlgorithm(algorithm);
}

void MainWindow::OnSchedulerJobFinished(AlgorithmModuleBase* algorithmModuleBase)
{
  delete algorithmModuleBase;
}

void MainWindow::OnAlgorithmFinished(AlgorithmModuleBase* algorithmModuleBase)
{
  if (algorithmModuleBase->GetAlgorithmNameShort() == "GeodesicTraining")
  {
#ifdef BUILD_MODULE_MitkImageViewer
    long uid = algorithmModuleBase->GetUid();

    auto segmentationIids = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
      uid, "Segmentation"
    );

    auto maskIids = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
      uid, "Mask"
    );

    // Find the most recent segmentation (Not needed now but that might change)
    // If the segmentations are saved with 2,3,etc in the end

    long recentSegmentationIid = (segmentationIids.size() > 0) ? segmentationIids[0] : -1;
    QString recentSegmentationPath = m_DataManager->GetDataPath(recentSegmentationIid);

    for (const long& segmentationIid : segmentationIids)
    {
      QString segmentationPath = m_DataManager->GetDataPath(segmentationIid);

      if (segmentationPath.endsWith(".nii.gz") &&
          segmentationPath > recentSegmentationPath
      ) {
        recentSegmentationIid  = segmentationIid;
        recentSegmentationPath = segmentationPath;
      }
    }

    long maskNrrdIid = -1;

    for (const long& maskIid : maskIids)
    {
      if (m_DataManager->GetDataPath(maskIid).endsWith(".nrrd"))
      {
        maskNrrdIid = maskIid;
        break;
      }
    }

    if (maskNrrdIid != -1 && recentSegmentationIid != -1)
    {
      qobject_cast<MitkImageViewer*>(m_ImageViewer)->ConvertToNrrdAndSave(
        recentSegmentationIid, maskNrrdIid
      );
    }
#endif
  }
}

void MainWindow::OnAlgorithmFinishedWithError(AlgorithmModuleBase* algorithmModuleBase, 
  QString errorMessage)
{
  QMessageBox::warning(
		this,
		algorithmModuleBase->GetAlgorithmName(),
		errorMessage	
	);
}

#ifdef BUILD_MITK
void MainWindow::OnSegmentationButtonClicked()
{
  this->m_SegmentationPanel->setVisible(true);
}
#endif

void MainWindow::SelectedSubjectChangedHandler(long uid)
{
	qDebug() << "Selected Subject Changed for MainWindow";

  // Checking if there is a mask already
  auto iids = m_DataManager->GetAllDataIdsOfSubject(uid);

  bool foundMask = false;
  for (const auto& iid : iids)
  {
    if (m_DataManager->GetDataSpecialRole(iid) == "Mask")
    {
      foundMask = true;
      break;
    }
  }

#ifdef BUILD_MITK
  if (foundMask) {
    this->m_SegmentationPanel->setVisible(true);
  }
  else {
    this->m_SegmentationPanel->setVisible(false);
  }
#endif 

}

// void MainWindow::EnableRunButton()
// {
// 	ui->pushButtonRun->setEnabled(true);

// 	QString styleSheet(" QPushButton { font:22px;color:black; background-color:lightblue; border-style: solid;");
// 	styleSheet += "border-color: black; border-width: 2px; border-radius: 10px; padding: 3px;}";
// 	styleSheet += "QPushButton:checked{ background-color: red; border-style: outset;";
// 	styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
// 	styleSheet += "QPushButton:hover{ background-color: lightGray; border-style: outset;";
// 	styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
// 	styleSheet += "QPushButton:pressed{ background-color: darkGray; border-style: outset;";
// 	styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";

// 	ui->pushButtonRun->setStyleSheet(styleSheet);

// 	// Shadow effect
// 	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
// 	effect->setBlurRadius(1);
// 	effect->setXOffset(1);
// 	effect->setYOffset(1);
// 	effect->setColor(Qt::black);
// 	ui->pushButtonRun->setGraphicsEffect(effect);
// }

// void MainWindow::DisableRunButton()
// {
//   ui->pushButtonRun->setEnabled(false);
  
//   QString styleSheet(" QPushButton { font:22px;color:rgba(255,255,255,135); background-color:rgba(255,255,255,20); border-style: solid;");
//   styleSheet += "border-color: darkGray; border-width: 1px; border-radius: 10px; padding: 3px;}";
//   //styleSheet += "QPushButton:checked{ background-color: rgba(0,125,0,0); border-style: outset;";
//   //styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
//   //styleSheet += "QPushButton:hover{ background-color: rgba(0,125,0,0); border-style: outset;";
//   //styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
//   //styleSheet += "QPushButton:pressed{ background-color: rgba(0,125,0,0); border-style: outset;";
//   //styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";

//   ui->pushButtonRun->setStyleSheet(styleSheet);

//   // Shadow effect
//   QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
//   effect->setBlurRadius(0);
//   effect->setXOffset(0);
//   effect->setYOffset(0);
//   effect->setColor(Qt::lightGray);
//   ui->pushButtonRun->setGraphicsEffect(effect);
// }
