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

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
  m_Scheduler.Start(); 
  m_DataManager = new DataManager(this);
  m_DataManager->SetAcceptedFileTypes(m_AcceptedFileTypes);
  
  ui->setupUi(this);

  // Set the DataView
  m_DataView = new DataTreeView(ui->dataViewContainer);
  /*QGridLayout*/QHBoxLayout *layoutDataViewer = new QHBoxLayout(ui->dataViewContainer);
  layoutDataViewer->addWidget(m_DataView);
  m_DataView->SetDataManager(m_DataManager);

  // Initialize image viewer
#ifdef BUILD_VIEWER
  m_ImageViewer = new MitkViewer(ui->viewerContainer);
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
  m_Scheduler.connect(&m_Scheduler, SIGNAL(jobFinished(long)), 
    this, SLOT(SchedulerResultReady(long))
  );
  m_Scheduler.connect(&m_Scheduler, SIGNAL(updateProgress(long, int)), 
    m_DataView, SLOT(UpdateProgressHandler(long, int))
  );
  connect(ui->pushButtonRun, SIGNAL(released()), 
    this, SLOT(RunPressed())
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

void MainWindow::RunPressed()
{
	std::shared_ptr<Scheduler::Data> data(new Scheduler::Data());

	long uid = m_CurrentSubjectID; // For convenience

    qDebug() << QString("(Run) Added uid:  ") << QString::number(uid);
	data->uids.push_back(uid);
	data->patientDirectoryPath[uid] = m_DataManager->GetDataPath(uid).toStdString();

	std::vector<long> dataIdsOfSubject = m_DataManager->GetAllDataIdsOfSubject(uid);

	for (const long& iid : dataIdsOfSubject)
	{
		QString dataPath        = m_DataManager->GetDataPath(iid);
		QString dataSpecialRole = m_DataManager->GetDataSpecialRole(iid); 

		if (dataSpecialRole.toStdString() == "Mask") {
			data->maskPath[uid] = dataPath.toStdString();
		}
		else {
			data->imagesPaths[uid].push_back(dataPath.toStdString());
		}
	}

	if (data->imagesPaths.find(uid) == data->imagesPaths.end() || data->maskPath.find(uid) == data->maskPath.end()) {
		qDebug() << QString("(Run) Images or mask missing so the uid was dismissed");
		data->uids.pop_back();
	}

	if (data->uids.size() > 0) {
		qDebug() << QString("Trying to run");
		m_Scheduler.AddData(data);
	}
}

void MainWindow::SchedulerResultReady(long uid)
{
  qDebug() << QString("SchedulerResultReady called for uid: ") << QString::number(uid);

  QString segmPath = m_DataManager->GetSubjectPath(uid) + "/MPIP_output/labels_res.nii.gz";

  m_DataManager->AddDataToSubject(uid, segmPath, QString("Segmentation"), 
    QString(), QString("labels_res.nii.gz")
  );
}

void MainWindow::SelectedSubjectChangedHandler(long uid)
{
  m_CurrentSubjectID = uid;
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
