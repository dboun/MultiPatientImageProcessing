#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

#include <vector>

#ifdef BUILD_MITK
#include "MPIPQmitkSegmentationPanel.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
  // Initialize DataManager
  m_DataManager = new DataManager(this);
  m_DataManager->SetAcceptedFileTypes(m_AcceptedFileTypes);
  
  // Initialize Scheduler
#ifdef BUILD_GEODESIC_TRAINING
  m_Scheduler = new Scheduler(this);
#else
  m_Scheduler = new SchedulerBase(this);
#endif

  m_Scheduler->SetDataManager(m_DataManager);
  m_Scheduler->Start(); 
  
  // Initialize UI
  ui->setupUi(this);
  this->setWindowTitle( m_AppName );

  // Initialize DataView
  m_DataView = new DataTreeView(ui->dataViewContainer);
  m_DataView->setMinimumWidth(350);
  /*QGridLayout*/QHBoxLayout *layoutDataViewer = new QHBoxLayout(ui->dataViewContainer);
  layoutDataViewer->addWidget(m_DataView);
  m_DataView->SetDataManager(m_DataManager);

  // Initialize ImageViewer
#ifdef BUILD_MITK
  m_ImageViewer = new MitkViewer(ui->viewerContainer);
  m_ImageViewer->setMinimumWidth(600);
  m_ImageViewer->setMinimumHeight(500);
  this->m_SegmentationPanel = new MPIPQmitkSegmentationPanel(qobject_cast<MitkViewer*>(m_ImageViewer)->GetDataStorage(), this);
  this->m_SegmentationPanel->SetDataManager(m_DataManager);
  this->ui->rightPanel->layout()->addWidget(this->m_SegmentationPanel);
  this->m_SegmentationPanel->hide();
#else
  m_ImageViewer = new ImageViewerBase(ui->viewerContainer);
  //m_ImageViewer->setGeometry(0, 0, 300, 300);
  //m_ImageViewer->setStyleSheet("background-color:black");
  //m_ImageViewer->show();
#endif

  ui->viewerContainer->layout()->addWidget(m_ImageViewer);
  m_ImageViewer->SetDataManager(m_DataManager);
  m_ImageViewer->SetDataView(m_DataView);
  m_ImageViewer->SetOpacitySlider(ui->opacitySlider);

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
  connect(m_Scheduler, SIGNAL(updateProgress(long, int)), 
    m_DataView, SLOT(UpdateProgressHandler(long, int))
  );
  connect(m_Scheduler, SIGNAL(jobQueued(long)), 
    this, SLOT(OnSchedulerJobQueued(long))
  );
  // connect(m_Scheduler, SIGNAL(jobFinished(long)), 
  //   this, SLOT(OnSchedulerResultReady(long))
  // );
  connect(ui->pushButtonRun, SIGNAL(released()), 
    this, SLOT(OnRunPressed())
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

#ifdef BUILD_MITK
  connect(ui->SegmentationBtn, SIGNAL(clicked()), this, SLOT(OnSegmentationButtonClicked()));
  connect(this, SIGNAL(EnableSegmentation()), m_SegmentationPanel, SLOT(OnEnableSegmentation()));
  connect(this, SIGNAL(DisableSegmentation()), m_SegmentationPanel, SLOT(OnDisableSegmentation()));
  connect(m_ImageViewer, SIGNAL(DisplayedDataName(long)), m_SegmentationPanel, SLOT(SetDisplayDataName(long)));
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
    qDebug(("Dropped file:" + fileName).toStdString().c_str());

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
	std::shared_ptr<SchedulerBase::SchedulerJobData> data(new SchedulerBase::SchedulerJobData());

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

	std::vector<long> iidsOfSubject = m_DataManager->GetAllDataIdsOfSubject(uid);

	int numberOfImages = 0, numberOfMasks = 0;

	for (const long& iid : iidsOfSubject)
	{
		QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid); 

		if (dataSpecialRole == "Mask") {
			numberOfMasks++;
		}
		else if (dataSpecialRole != "Segmentation")
		{
			numberOfImages++;
		}
	}

	if (numberOfMasks == 0)
	{
		QMessageBox::information(
		  this,
		  tr("No mask drawn"),
		  tr("Please create a mask and run again.")
		);
		return;
	}
	if (numberOfMasks > 1)
	{
		QMessageBox::information(
			this,
			tr("Multiple masks"),
			tr("Please remove all but one masks (Right click & Remove won't delete an image from the disk).")
		);
		return;
	}
	if (numberOfImages == 0)
	{
		QMessageBox::information(
			this,
			tr("No images"),
			tr("Please load some images.")
		);
		return;
	}

	data->uids.push_back(uid);
	data->iids[uid] = iidsOfSubject;

	qDebug() << QString("Trying to run");
	m_Scheduler->AddData(data);
}

// void MainWindow::OnSchedulerResultReady(long uid)
// {
//   qDebug() << QString("OnSchedulerResultReady called for uid: ") << QString::number(uid);
// }

void MainWindow::OnSchedulerJobQueued(long uid)
{
	m_DataView->UpdateProgressHandler(uid, 0);
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
