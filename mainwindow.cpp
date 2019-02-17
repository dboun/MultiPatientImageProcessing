#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
  
{
  m_Scheduler.Start(); 
  ui->setupUi(this);

#ifdef BUILD_VIEWER
  qDebug() << "Using viewer";
  //m_Viewer = new VtkViewer();
  m_Viewer = new MpipMitkViewer();
	
  // Replace viewerContainer with viewer of choice
  QGridLayout *layout = new QGridLayout(ui->viewerContainer);
  layout->addWidget(m_Viewer, 0, 0);
#endif

  // Disable unused buttons
  ui->actionAdd_image_for_selected_subject->setVisible(false);
  ui->actionAdd_image_for_new_subject->setVisible(false);
  ui->actionAdd_multiple_subjects->setVisible(false);
  ui->pushButtonConfigure->setVisible(false);

  // Turn drag and drop on
  setAcceptDrops(true); 

  // TreeWidget columns
  QStringList columnNames = QStringList() << "  Select subjects"; // << "Progress";
  ui->patientTree->setHeaderLabels(columnNames);
  ui->patientTree->setColumnCount(1);

  // Shadow effect
  QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
  effect->setBlurRadius(1);
  effect->setXOffset(1);
  effect->setYOffset(1);
  effect->setColor(Qt::black);
  ui->pushButtonRun->setGraphicsEffect(effect);
  ui->patientTree->setGraphicsEffect(effect);

  // Signals and Slots
  m_Scheduler.connect(&m_Scheduler, SIGNAL(jobFinished(long)), this, SLOT(SchedulerResultReady(long)));
  m_Scheduler.connect(&m_Scheduler, SIGNAL(updateProgress(long, int)), this, SLOT(UpdateProgress(long, int)));
  connect(ui->pushButtonRun, SIGNAL(released()), this, SLOT(RunPressed()));
  connect(ui->actionOpen_single_subject, SIGNAL(triggered()), this, SLOT(OnOpenSingleSubject()));
  connect(ui->pushButtonConfigure, SIGNAL(released()), this, SLOT(handleConfigButton()));

  connect(ui->patientTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
    this, SLOT(OnTreeWidgetClicked(QTreeWidgetItem*, int))
  );

  ui->patientTree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->patientTree, SIGNAL(customContextMenuRequested(const QPoint&)),
    this, SLOT(ShowTreeContextMenu(const QPoint&))
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

    LoadSingleSubject(fileName);
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
    "/home",
    QFileDialog::ShowDirsOnly |
    QFileDialog::DontResolveSymlinks
  );

  if (!dir.isEmpty() && LoadSingleSubject(dir))
  {
	SwitchExpandedView(ui->patientTree->topLevelItem(ui->patientTree->topLevelItemCount() - 1));
	// TODO : Display the first image
  }
}

void MainWindow::handleConfigButton()
{
  // TODO (Not implemented yet)
  QStringList apps;

//#ifdef BUILD_GEODESIC_TRAINING
  //apps << "Geodesic Training Segmentation";
//#endif // BUILD_GEODESIC_TRAINING

}

void MainWindow::OnTreeWidgetClicked(QTreeWidgetItem *item, int column)
{
  qDebug() << QString("Clicked tree item.");
  
  // Make only one expanded
  QTreeWidgetItem* currentSelected = ui->patientTree->currentItem();
  QTreeWidgetItem* currentTopLevelItem = (
	  (currentSelected->childCount() == 0 && currentSelected->parent()) ?
	    currentSelected->parent() :
	    currentSelected
  );
  
  if (currentSelected->childCount() > 0) {
	SwitchExpandedView(currentTopLevelItem);
  }

  // If it's a child item and it got checked
  if (item->childCount() == 0 && item->checkState(0) == Qt::Checked)
  {
    //item->parent()->setCheckState(0, Qt::Checked);
	SwitchSubjectAndImage(
		item->parent()->data(0, PATIENT_UID_ROLE).toLongLong(),  
		item->data(0, IMAGE_PATH_ROLE).toString()
	);
  }
  
  //// If it's a child item and now the parent has only unchecked items
  //if (item->childCount() == 0 && item->checkState(0) == Qt::Unchecked && item->parent()->checkState(0) == Qt::Checked) {
  //  bool foundChecked = false;

  //  for (int i = 0; i < item->parent()->childCount(); i++) {
  //    if (item->parent()->child(i)->checkState(0) == Qt::Checked) {
  //      foundChecked = true;
  //      qDebug() << QString("Found checked.");
  //      break;
  //    }
  //  }

  //  if (!foundChecked) {
  //    item->parent()->setCheckState(0, Qt::Unchecked);
  //  }
  //}
}

void MainWindow::ShowTreeContextMenu(const QPoint& pos)
{
  qDebug() << QString("Show context menu pressed");

  if (ui->patientTree->itemAt(pos)) {

    QMenu *contextMenu = new QMenu(ui->patientTree);
    
    QAction action1("Remove", this);
    //action1.setShortcutContext(Qt::ApplicationShortcut);
    //action1.setShortcut(QKeySequence::Delete);
    connect(&action1, SIGNAL(triggered()), this, SLOT(TreeContextRemoveItem()));
    contextMenu->addAction(&action1);
    
    QAction action2("Set as mask", this);
    connect(&action2, SIGNAL(triggered()), this, SLOT(TreeContextSetItemAsMask()));

    if (ui->patientTree->itemAt(pos)->childCount() == 0) {
      // The item is an image
      qDebug() << ui->patientTree->itemAt(pos)->data(0, IMAGE_PATH_ROLE);

      contextMenu->addAction(&action2);
    }
    
    contextMenu->exec(ui->patientTree->viewport()->mapToGlobal(pos));
  }
}

void MainWindow::TreeContextRemoveItem()
{
  qDebug() << QString("Trying to delete item");

  if (ui->patientTree->currentItem()->childCount() == 0 &&
	  ui->patientTree->currentItem()->parent()->childCount() == 1)
  {
	  delete ui->patientTree->currentItem()->parent();
  }
  else {
	delete ui->patientTree->currentItem();
  }

  if (ui->patientTree->topLevelItemCount() > 0)
  {
	  SwitchExpandedView(ui->patientTree->topLevelItem(0));
  }
  //this->ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::TreeContextSetItemAsMask()
{
  for (int i = 0; i < ui->patientTree->currentItem()->parent()->childCount(); i++) {
    ui->patientTree->currentItem()->parent()->child(i)->setText(0, 
      ui->patientTree->currentItem()->parent()->child(i)->data(0, IMAGE_NAME_ROLE).toString()
    );
  }

  ui->patientTree->currentItem()->setText(0, QString("<Mask>"));
}

void MainWindow::RunPressed()
{
	std::shared_ptr<Scheduler::Data> data(new Scheduler::Data());

	QTreeWidgetItem* selectedSubject;
	QTreeWidgetItem* currentItem = ui->patientTree->currentItem();
	
	if (!currentItem) { return; }

	if (currentItem->childCount() == 0) { 
		selectedSubject = currentItem->parent(); 
	}
	else {
		selectedSubject = currentItem;
	}
  
	long uid = selectedSubject->data(0, PATIENT_UID_ROLE).toLongLong();
	qDebug() << QString("(Run) Added uid:  ") << QString(uid);
	data->uids.push_back(uid);
	data->patientDirectoryPath[uid] = selectedSubject->data(0, IMAGE_PATH_ROLE).toString().toStdString();

	for (int i = 0; i < selectedSubject->childCount(); i++)
	{
		QTreeWidgetItem* child = selectedSubject->child(i);
		
		if (child->checkState(0) == Qt::Checked) {
			if (child->text(0).toStdString() == "<Mask>") {
				data->maskPath[uid] = child->data(0, IMAGE_PATH_ROLE).toString().toStdString();
			}
			else {
				data->imagesPaths[uid].push_back(child->data(0, IMAGE_PATH_ROLE).toString().toStdString());
			}
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
  qDebug() << QString("SchedulerResultReady called for uid: ") << QString(uid);

  if (subjectByUid[uid])
  {
    //subjectByUid[uid]->setText(0, subjectByUid[uid]->text(0).append(QString(" [FINISHED]")));

    QTreeWidgetItem *imageItem = new QTreeWidgetItem(subjectByUid[uid]);

    QString file = subjectByUid[uid]->data(0, IMAGE_PATH_ROLE).toString() + QString("/MPIP_output/labels_res.nii.gz");

    imageItem->setText(0, QString("<Segmentation>"));
    imageItem->setCheckState(0, Qt::Checked);

    imageItem->setData(0, IMAGE_PATH_ROLE, file);          // path to the image
    imageItem->setData(0, IMAGE_NAME_ROLE, QString("<Segmentation>"));
  }
}

void MainWindow::UpdateProgress(long uid, int progress)
{
  QProgressBar *progressBar = subjectByUid[uid]->treeWidget()->findChild<QProgressBar*>(QString("ProgressBar") + QString(uid));
  progressBar->setVisible(true);
  progressBar->setValue(progress);
}

void MainWindow::Load(QString filePath, QString overlayPath)
{
  qDebug() << QString("LOADING: ") << filePath << QString(", with overlay: ") << overlayPath;
#ifdef BUILD_VIEWER
  m_Viewer->Display(filePath, overlayPath);
#endif
}

bool MainWindow::LoadSingleSubject(QString directoryPath)
{
  std::lock_guard<std::mutex> lg(m_TreeEditMutex); // Lock mutex (gets unlocked on function finishing)
  qDebug() << QString("Load single subject root: ") << directoryPath;

  QStringList files;
  LoadAllFilesRecursive(directoryPath, files);

  if (files.isEmpty()) {
    qDebug(std::string("No data found for subject").c_str());
    return false;
  }
  
  // Update tree widget
  QTreeWidgetItem *patientToAdd = new QTreeWidgetItem(ui->patientTree);
  patientToAdd->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  
  QString patientName = QString::fromStdString(
    directoryPath.toStdString().substr(
      directoryPath.toStdString().find_last_of("/\\") + 1
    )
  );

  //patientToAdd->setText(0, patientName);
  //patientToAdd->setCheckState(0, Qt::Checked);
  patientToAdd->setSelected(true);
  patientToAdd->setData(0, IMAGE_PATH_ROLE, directoryPath);
  patientToAdd->setData(0, IMAGE_NAME_ROLE, patientName);
  patientToAdd->setData(0, PATIENT_UID_ROLE, uidNextToGive);
  subjectByUid[uidNextToGive] = patientToAdd;

  QProgressBar *progressBar = new QProgressBar();
  progressBar->setObjectName(QString("ProgressBar") + QString(uidNextToGive));
  progressBar->setVisible(false);
  progressBar->setMinimum(0);
  progressBar->setMaximum(100);
  progressBar->setValue(0);
  progressBar->setAlignment(Qt::AlignCenter);
  progressBar->setMinimumWidth(30);
  progressBar->setMaximumWidth(40);
  progressBar->setMaximumHeight(20);
  progressBar->setStyleSheet("QProgressBar {"
    "background-color: #4f4f4f;"
    "color: #9a9a9a;"
    "border-style: outset;"
    "border-width: 2px;"
    "border-color: #BF360C;"
    "border-radius: 4px;"
    "text-align: center; }"

    "QProgressBar::chunk {"
    "background-color: #F5F5F5; }"
  );
  //"border-color: #4FC3F7;"
  //"QProgressBar::chunk {"
  //"background-color: #010327; }"

  QLabel *label = new QLabel(patientName);
  label->setMaximumHeight(20);
  label->setAttribute(Qt::WA_TransparentForMouseEvents, true);

  QWidget *labelAndProgress = new QWidget();
  labelAndProgress->setStyleSheet("background-color: rgba(0,0,0,0)");
  labelAndProgress->setAutoFillBackground(true);
  labelAndProgress->setMaximumHeight(35);
  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->addWidget(label, Qt::AlignLeft);
  hLayout->addWidget(progressBar, Qt::AlignRight);
  labelAndProgress->setLayout(hLayout);
  labelAndProgress->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  
  //patientToAdd->setText(0, patientName);
  ui->patientTree->setItemWidget(patientToAdd, 0, labelAndProgress);

  foreach(QString file, files) {
    QTreeWidgetItem *imageItem = new QTreeWidgetItem(patientToAdd);
    
    QString shortName = QString::fromStdString(
      file.toStdString().substr(
        file.toStdString().find_last_of("/\\") + 1
      )
    );
    
    imageItem->setText(0, shortName);
    imageItem->setCheckState(0, Qt::Checked);

    imageItem->setData(0, IMAGE_PATH_ROLE, file);          // path to the image
    imageItem->setData(0, IMAGE_NAME_ROLE, shortName); // short name
  }

  uidNextToGive++;

  return true;
}

void MainWindow::LoadAllFilesRecursive(QString directoryPath, QStringList& allFiles)
{
  qDebug() << QString("Trying dir: ") << directoryPath;
  QDir dir = QDir(directoryPath);

  // Find all files in this directory
  QStringList files = dir.entryList(m_AcceptedFileTypes,
    QDir::Files | QDir::NoSymLinks);

  // Find all subdirectories
  dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList subdirectories = dir.entryList();

  // Add all files to the patient list
  for (const auto& file : files)
  {
    allFiles << directoryPath + QString("/") + file;
  }

  // Do the same for subdirectories
  for (const auto& subdir : subdirectories)
  {
    //qDebug(subdir.toStdString().c_str());
    LoadAllFilesRecursive(directoryPath + QString("/") + subdir, allFiles);
  }
}

void MainWindow::SwitchSubjectAndImage(long uid, QString imagePath)
{
	qDebug() << QString("Switch subject ") << QString::number(uid) << QString(", imagePath=") << imagePath;

	QTreeWidgetItem* currentPatient = subjectByUid[uid];
	bool loadCalled = false;

	for (int i = 0; i < currentPatient->childCount(); i++) {
		QTreeWidgetItem* currentImage = currentPatient->child(i);
		//QString currentImageName = currentImage->data(0, IMAGE_NAME_ROLE).toString();
		QString currentImageText = currentImage->text(0);
		//qDebug() << QString("Checking ") << currentImageName;

		if (currentImage->checkState(0) == Qt::Checked && 
			currentImageText.startsWith(QString("<Mask"))) 
		{
			QString currentImagePath = currentImage->data(0, IMAGE_PATH_ROLE).toString();
			Load(imagePath, currentImagePath);
			loadCalled = true;
			break;
		}
	}

	if (!loadCalled) {
		if (!imagePath.isEmpty())
		{
			Load(imagePath);
		}
		else {
			if (currentPatient->childCount() > 0) { Load(currentPatient->child(0)->data(0, IMAGE_PATH_ROLE).toString()); }
		}
	}

}

void MainWindow::SwitchExpandedView(QTreeWidgetItem* focusItem)
{
	// We are concerned about top level items (subjects)
	if (focusItem && focusItem->childCount() == 0)
	{
		focusItem = focusItem->parent();
	}

	if (focusItem)
	{
		focusItem->setSelected(true);
		focusItem->setExpanded(true);
		int focusItemIndex = ui->patientTree->indexOfTopLevelItem(focusItem);

		for (int i = 0; i < ui->patientTree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* topLevelItem = ui->patientTree->topLevelItem(i);
			
			if (i != focusItemIndex)
			{
				qDebug() << "Un-expanding item";
				ui->patientTree->topLevelItem(i)->setSelected(false);
				ui->patientTree->topLevelItem(i)->setExpanded(false);
			}
		}
	}
}

void MainWindow::DisableRunButton()
{
	ui->pushButtonRun->setEnabled(false);
	
	QString styleSheet(" QPushButton { font:22px;color:rgba(255,255,255,135); background-color:rgba(255,255,255,20); border-style: solid;");
	styleSheet += "border-color: darkGray; border-width: 1px; border-radius: 10px; padding: 3px;}";
	//styleSheet += "QPushButton:checked{ background-color: rgba(0,125,0,0); border-style: outset;";
	//styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
	//styleSheet += "QPushButton:hover{ background-color: rgba(0,125,0,0); border-style: outset;";
	//styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
	//styleSheet += "QPushButton:pressed{ background-color: rgba(0,125,0,0); border-style: outset;";
	//styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";

	ui->pushButtonRun->setStyleSheet(styleSheet);

	// Shadow effect
	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
	effect->setBlurRadius(0);
	effect->setXOffset(0);
	effect->setYOffset(0);
	effect->setColor(Qt::lightGray);
	ui->pushButtonRun->setGraphicsEffect(effect);
}

void MainWindow::EnableRunButton()
{
	ui->pushButtonRun->setEnabled(true);

	QString styleSheet(" QPushButton { font:22px;color:black; background-color:lightblue; border-style: solid;");
	styleSheet += "border-color: black; border-width: 2px; border-radius: 10px; padding: 3px;}";
	styleSheet += "QPushButton:checked{ background-color: red; border-style: outset;";
	styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
	styleSheet += "QPushButton:hover{ background-color: lightGray; border-style: outset;";
	styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";
	styleSheet += "QPushButton:pressed{ background-color: darkGray; border-style: outset;";
	styleSheet += "border-style: solid; border-color: black; border-width: 2px; border-radius: 10px; }";

	ui->pushButtonRun->setStyleSheet(styleSheet);

	// Shadow effect
	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
	effect->setBlurRadius(1);
	effect->setXOffset(1);
	effect->setYOffset(1);
	effect->setColor(Qt::black);
	ui->pushButtonRun->setGraphicsEffect(effect);
}