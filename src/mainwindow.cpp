#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextStream>
#include <QListView>
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
  m_DataView = new DataTreeView(this);
  //m_DataView->setMinimumWidth(400);
  GuiModuleBase::PlaceWidgetInWidget(m_DataView, ui->dataViewContainer);
  m_DataView->SetDataManager(m_DataManager);
  m_DataView->SetAppName(m_AppName);
  m_DataView->SetAppNameShort(m_AppNameShort);

  // Initialize ImageViewer
#ifdef BUILD_MODULE_MitkImageViewer
  m_ImageViewer = new MitkImageViewer(ui->viewerContainer);
  m_ImageViewer->setMinimumWidth(600);
  m_ImageViewer->setMinimumHeight(500);

  connect(m_DataView, SIGNAL(ExportData(long, QString)), 
    qobject_cast<MitkImageViewer*>(m_ImageViewer), SLOT(OnExportData(long, QString))
  );
#else
  qDebug() << "Using abstract image viewer";
  m_ImageViewer = new ImageViewerBase(ui->viewerContainer);
#endif

  //ui->viewerContainer->layout()->addWidget(m_ImageViewer);
  GuiModuleBase::PlaceWidgetInWidget(m_ImageViewer, ui->viewerContainer);
  m_ImageViewer->SetDataManager(m_DataManager);
  m_ImageViewer->SetDataView(m_DataView);
  m_ImageViewer->SetOpacitySlider(ui->opacitySlider);
  m_ImageViewer->SetAppName(m_AppName);
  m_ImageViewer->SetAppNameShort(m_AppNameShort);

  // Initialize MitkDrawingTool
#ifdef BUILD_MODULE_MitkDrawingTool
  m_MitkDrawingTool = new MitkDrawingTool(
    qobject_cast<MitkImageViewer*>(m_ImageViewer)->GetDataStorage(), ui->drawingToolContainer
  );

  m_MitkDrawingTool->SetMitkImageViewer(
    qobject_cast<MitkImageViewer*>(m_ImageViewer)
  );

  m_MitkDrawingTool->SetDataView(m_DataView);
  m_MitkDrawingTool->SetDataManager(m_DataManager);
  m_MitkDrawingTool->SetAppName(m_AppName);
  m_MitkDrawingTool->SetAppNameShort(m_AppNameShort);
  //this->ui->rightPanel->layout()->addWidget(this->m_MitkDrawingTool);
  GuiModuleBase::PlaceWidgetInWidget(m_MitkDrawingTool, ui->drawingToolContainer);
  //this->m_SegmentationPanel->hide();
#endif

  // Disable unused buttons
  // ui->actionAdd_image_for_selected_subject->setVisible(false);
  // ui->actionAdd_image_for_new_subject->setVisible(false);
  // ui->actionAdd_multiple_subjects->setVisible(false);

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
  connect(ui->actionOpen_Subjects, SIGNAL(triggered()), 
    this, SLOT(OnOpenSubjects())
  );
  connect(ui->actionAdd_image_for_new_subject, SIGNAL(triggered()), 
    this, SLOT(OnOpenImagesForNewSubject())
  );
  connect(ui->actionAdd_image_for_selected_subject, SIGNAL(triggered()), 
    this, SLOT(OnOpenImagesForSelectedSubject())
  );
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
  QStringList filesOrDirs;
  bool foundFile = false, foundDir = false;

  foreach(const QUrl &url, e->mimeData()->urls()) {
    QString fileName = url.toLocalFile();
    filesOrDirs.push_back(fileName);

    QFileInfo fileCheck(fileName);

    if (fileCheck.isDir())
    {
      foundDir = true;

      if (foundFile) { break; }
    }
    else {
      foundFile = true;

      if (foundDir) { break; }
    }

    qDebug() << "Dropped file:" + fileName;
  }

  if (foundFile && foundDir)
  {
    QMessageBox::information(
      this,
      m_AppNameShort,
      tr("Please drag and drop either folders where each one is a subject, or multiple images of one subject.")
    );    
    return;
  }

  if (foundDir)
  {
    foreach(const QString dir, filesOrDirs)
    {
      m_DataManager->AddSubjectAndDataByDirectoryPath(dir);
    }
  }
  else {
    QString firstFileBaseName = QFileInfo(filesOrDirs.at(0)).baseName();
    QStringList firstFileSplit = firstFileBaseName.split("_");
    QString divider = "_";
    
    // If the name is not using '_', assume '-'
    if (firstFileSplit.size() == 1) { 
      firstFileSplit = firstFileBaseName.split("-"); 
      divider = "-";
    }

    QString suggestedSubjectName = firstFileSplit.at(0);
    for (int i=1; i < firstFileSplit.size()-1; i++)
    {
      suggestedSubjectName = suggestedSubjectName + divider + firstFileSplit.at(i);
    }


    // Get Subject Name
    QString subjectName;
    bool ok;
    do {
      subjectName = QInputDialog::getText(this, 
        m_AppNameShort,
        tr("Subject name:"), 
        QLineEdit::Normal,
        suggestedSubjectName, 
        &ok
      );
    } while(ok && subjectName.isEmpty());
    
    if (!ok)
    {
      // The user pressed cancel
      return;
    }

    // Add the data to a new subject
    long uid = m_DataManager->AddSubject(
      QFileInfo(filesOrDirs.at(0)).absoluteDir().path(), subjectName
    );

    foreach(const QString file, filesOrDirs)
    {
      m_DataManager->AddDataToSubject(uid, file);
    }
  }

  //// Not implemented yet message
  //QMessageBox::information(
  //  this,
  //  tr("MPIP"),
  //  tr("Drag and drop is not fully implemented yet.")
  //);
}

void MainWindow::closeEvent (QCloseEvent *e)
{
    // QMessageBox::StandardButton resBtn = QMessageBox::question( this, APP_NAME,
    //   tr("Are you sure?\n"),
    //   QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
    //   QMessageBox::Yes
    // );
    // if (resBtn != QMessageBox::Yes) {
    //     e->ignore();
    // } else {
    //     e->accept();
    // }

  if (!m_Scheduler->IsSafeToExit())
  {
    QMessageBox::information( this, 
      m_AppNameShort,
      tr("Please wait for all operations to finish.")
    );
    e->ignore();
    return;
  }

  // Remove data in reverse order to avoid reloading everything to the viewer
	auto uids = m_DataManager->GetAllSubjectIds();
	
  // Test
	{
		for(auto uid : uids)
		{
			qDebug() << "uid" << uid << "was loaded";
		}
	}
  
  long uid;
	for (long i = uids.size()-1; i >= 0; i--)
	{
		uid = uids[i];
    qDebug() << "MainWindow::closeEvent Asking uid" << uid << "to be removed - i=" << i;
		m_DataManager->RemoveSubject(uid);
	}

  //e->accept();
}

void MainWindow::OnOpenSubjects()
{
  // Dialog to get the directories
  QFileDialog dialog(this);
  dialog.setDirectory(m_MostRecentDir);
  dialog.setFileMode(QFileDialog::DirectoryOnly);
  dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  dialog.setWindowTitle("Choose one or more directories (subjects)");

  // The following things make the dialog be able to
  // select multiple subjects because Qt doesn't support it as an option
  dialog.setOption(QFileDialog::DontUseNativeDialog, true);
  QListView *l = dialog.findChild<QListView*>("listView");
  if (l) {
    l->setSelectionMode(QAbstractItemView::MultiSelection);
  }
  QTreeView *t = dialog.findChild<QTreeView*>();
  if (t) {
    t->setSelectionMode(QAbstractItemView::MultiSelection);
  }

  // Show the dialog
  if (dialog.exec())
  {
    QStringList dirNames = dialog.selectedFiles();

    // Resolve a bug where sometimes the parent directory gets passed too
    if (dirNames.size() > 1)
    {
      QDir d = QDir(dirNames.at(1)); // The second dir in the list
      d.cdUp(); // go to parent of the second dir

      if (dirNames.at(0) == d.path())
      {
        dirNames.removeAt(0);
      }
    }

    // Add the subjects
    bool foundEmpty = false;
    foreach (QString dir, dirNames)
    {
      if (!dir.isEmpty())
      {
        // DataManager will notify everything that there was a change
        m_DataManager->AddSubjectAndDataByDirectoryPath(dir);
        m_MostRecentDir = dir;
      }
      else {
        foundEmpty = true;
      }
    }

    if (foundEmpty)
    {
      QMessageBox::information(this, m_AppName,
        "One or more of the directories was empty."
      );
    }
  }

  // QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
  //   m_MostRecentDir,
  //   QFileDialog::ShowDirsOnly |
  //   QFileDialog::DontResolveSymlinks
  // );

  // m_MostRecentDir = dir;
}

void MainWindow::OnOpenImagesForNewSubject()
{
  QStringList filenames = QFileDialog::getOpenFileNames(this,
    tr("Select images"),
    m_MostRecentDir,
    tr("Nifti images (*.nii.gz)") 
  );

  if (filenames.isEmpty())
  {
    return;
  }

  // Find suggested subject name

  QString firstFileBaseName = QFileInfo(filenames.at(0)).baseName();
  QStringList firstFileSplit = firstFileBaseName.split("_");
  QString divider = "_";
    
  // If the name is not using '_', assume '-'
  if (firstFileSplit.size() == 1) { 
    firstFileSplit = firstFileBaseName.split("-"); 
    divider = "-";
  }

  QString suggestedSubjectName = firstFileSplit.at(0);
  for (int i=1; i < firstFileSplit.size()-1; i++)
  {
    suggestedSubjectName = suggestedSubjectName + divider + firstFileSplit.at(i);
  }

  // Get Subject Name
  QString subjectName;
  bool ok;
  do {
    subjectName = QInputDialog::getText(this, 
      m_AppNameShort,
      tr("Subject name:"), 
      QLineEdit::Normal,
      suggestedSubjectName, 
      &ok
    );
  } while(ok && subjectName.isEmpty());
  
  if (!ok)
  {
    // The user pressed cancel
    return;
  }

  // Add the data to a new subject
  long uid = m_DataManager->AddSubject(
    QFileInfo(filenames.at(0)).absoluteDir().path(), subjectName
  );

  foreach(QString file, filenames)
  {
    m_DataManager->AddDataToSubject(uid, file);
  }
}

void MainWindow::OnOpenImagesForSelectedSubject()
{
  long uid = m_DataView->GetCurrentSubjectID(); 
  if (uid == -1)
  {
		QMessageBox::information(
			this,
			tr("No selected subject"),
			tr("Please load a subject by directory or images for new subject.")
		);
    return;
  }

  QStringList filenames = QFileDialog::getOpenFileNames(this,
    tr("Select images"),
    m_DataManager->GetOriginalSubjectPath(uid),
    tr("Nifti images (*.nii.gz)") 
  );

  foreach(QString file, filenames)
  {
    m_DataManager->AddDataToSubject(uid, file);
  }
}

void MainWindow::OnRunPressed()
{
  qDebug() << "MainWindow::OnRunPressed";
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

  if (m_SubjectsThatAreRunning.count(uid) != 0)
  {
    QMessageBox::warning(this,
      "Please wait",
      "Algorithm is already running for this subject"	
    );
    return;
  }

  std::unique_lock<std::mutex> ul(*m_DataManager->GetSubjectEditMutexPointer(uid));
	
  qDebug() << QString("(Run) uid:  ") << QString::number(uid);

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
    qobject_cast<MitkImageViewer*>(m_ImageViewer)->SaveImageToFile(maskNrrd, false);
  }

  // Remove all previous masks (if they exist)
  for (const long& iid : iids)
  {
    if (iid != maskNrrd)
    {
      m_DataManager->RemoveData(iid);
    }
  }  

  // Remove all previous segmentations (if they exist)
  for (const long& iid : m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Segmentation"))
  {
    m_DataManager->RemoveData(iid);
  }
#endif

#ifdef BUILD_MODULE_GeodesicTraining
	AlgorithmModuleBase* algorithm = new GeodesicTrainingModule();
#else
  AlgorithmModuleBase* algorithm = new AlgorithmModuleBase();
#endif

  m_SubjectsThatAreRunning.insert(uid);

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

  ul.unlock();
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
    // long uid = algorithmModuleBase->GetUid();

    // auto segmentationIids = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
    //   uid, "Segmentation"
    // );

    // auto maskIids = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
    //   uid, "Mask"
    // );

    // // Find the most recent segmentation (Not needed now but that might change)
    // // If the segmentations are saved with 2,3,etc in the end

    // long recentSegmentationIid = (segmentationIids.size() > 0) ? segmentationIids[0] : -1;
    // QString recentSegmentationPath = m_DataManager->GetDataPath(recentSegmentationIid);

    // for (const long& segmentationIid : segmentationIids)
    // {
    //   QString segmentationPath = m_DataManager->GetDataPath(segmentationIid);

    //   if (segmentationPath.endsWith(".nii.gz") &&
    //       segmentationPath > recentSegmentationPath
    //   ) {
    //     recentSegmentationIid  = segmentationIid;
    //     recentSegmentationPath = segmentationPath;
    //   }
    // }

    // long maskNrrdIid = -1;

    // for (const long& maskIid : maskIids)
    // {
    //   if (m_DataManager->GetDataPath(maskIid).endsWith(".nrrd"))
    //   {
    //     maskNrrdIid = maskIid;
    //     break;
    //   }
    // }

    // if (maskNrrdIid != -1 && recentSegmentationIid != -1)
    // {
    //   qobject_cast<MitkImageViewer*>(m_ImageViewer)->ConvertToNrrdAndSave(
    //     recentSegmentationIid, maskNrrdIid
    //   );
    // }
#endif

    m_SubjectsThatAreRunning.erase(algorithmModuleBase->GetUid());
  }
}

void MainWindow::OnAlgorithmFinishedWithError(AlgorithmModuleBase* algorithmModuleBase, 
  QString errorMessage)
{
  m_SubjectsThatAreRunning.erase(algorithmModuleBase->GetUid());
  QMessageBox::warning(
		this,
		algorithmModuleBase->GetAlgorithmName(),
		errorMessage	
	);
}

void MainWindow::SelectedSubjectChangedHandler(long uid)
{
	qDebug() << "Selected Subject Changed for MainWindow";

  if (uid == -1)
  {
    return;
  }

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
