#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
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
	m_Scheduler.connect(&m_Scheduler, SIGNAL(jobFinished(long)), this, SLOT(SchedulerResultReady(long)));

	ui->patientTree->setHeaderLabel("Select subjects");

	connect(ui->pushButtonRun, SIGNAL(released()), this, SLOT(RunPressed()));
    connect(ui->actionOpen_Dicom,SIGNAL(triggered()),this,SLOT(OnOpenDicom()));
    connect(ui->actionDisplay_Metadata,SIGNAL(triggered()),this,SLOT(OnDisplayDicomMetaData()));
	connect(ui->actionOpen_single_subject, SIGNAL(triggered()), this, SLOT(OnOpenSingleSubject()));
	connect(ui->pushButtonConfigure, SIGNAL(released()), this, SLOT(handleConfigButton()));

	connect(ui->patientTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(OnTreeWidgetClicked(QTreeWidgetItem*, int))
	);

	ui->patientTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->patientTree, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(ShowTreeContextMenu(const QPoint&))
	);

	// Shadow effect on Run button
	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
	effect->setBlurRadius(1);
	effect->setXOffset(1);
	effect->setYOffset(1);
	effect->setColor(Qt::black);

	ui->pushButtonRun->setGraphicsEffect(effect);
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

		LoadSingleSubject(fileName);
	}

	//// Not implemented yet message
	//QMessageBox::information(
	//	this,
	//	tr("MPIP"),
	//	tr("Drag and drop is not fully implemented yet.")
	//);
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

void MainWindow::OnOpenSingleSubject()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks
	);

	if (!dir.isEmpty() && LoadSingleSubject(dir))
	{
		SwitchSubjectAndImage(ui->patientTree->topLevelItemCount() - 1);
	}
}

void MainWindow::handleConfigButton()
{
	QStringList apps;

//#ifdef BUILD_GEODESIC_TRAINING
	//apps << "Geodesic Training Segmentation";
//#endif // BUILD_GEODESIC_TRAINING


}

void MainWindow::OnTreeWidgetClicked(QTreeWidgetItem *item, int column)
{
	qDebug() << QString("Clicked tree column: ") << QString(column);
	
	// If it's a parent item
	for (int i = 0; i < item->childCount(); i++) {
		item->child(i)->setCheckState(0, item->checkState(0));
	}

	// If it's a child item and it got checked
	if (item->childCount() == 0 && item->checkState(0) == Qt::Checked)
	{
		item->parent()->setCheckState(0, Qt::Checked);
		Load(item->data(0, Qt::UserRole).toString());
	}
	
	// If it's a child item and now the parent has only unchecked items
	if (item->childCount() == 0 && item->checkState(0) == Qt::Unchecked && item->parent()->checkState(0) == Qt::Checked) {
		bool foundChecked = false;

		for (int i = 0; i < item->parent()->childCount(); i++) {
			if (item->parent()->child(i)->checkState(0) == Qt::Checked) {
				foundChecked = true;
				qDebug() << QString("Found checked.");
				break;
			}
		}

		if (!foundChecked) {
			item->parent()->setCheckState(0, Qt::Unchecked);
		}
	}
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
			qDebug() << ui->patientTree->itemAt(pos)->data(0, Qt::UserRole);

			contextMenu->addAction(&action2);
		}
		
		contextMenu->exec(ui->patientTree->viewport()->mapToGlobal(pos));
	}
}

void MainWindow::TreeContextRemoveItem()
{
	qDebug() << QString("Trying to delete item");
	delete ui->patientTree->currentItem();
}

void MainWindow::TreeContextSetItemAsMask()
{
	for (int i = 0; i < ui->patientTree->currentItem()->parent()->childCount(); i++) {
		ui->patientTree->currentItem()->parent()->child(i)->setText(0, 
			ui->patientTree->currentItem()->parent()->child(i)->data(0, Qt::UserRole + 1).toString()
		);
	}

	ui->patientTree->currentItem()->setText(0, QString("<Mask>"));
}

void MainWindow::RunPressed()
{
	std::shared_ptr<Scheduler::Data> data(new Scheduler::Data());

	for (int i = 0; i < ui->patientTree->topLevelItemCount(); i++)
	{
		if (ui->patientTree->topLevelItem(i)->checkState(0) == Qt::Checked)
		{
			long uid = ui->patientTree->topLevelItem(i)->data(0, Qt::UserRole + 1).toLongLong();
			qDebug() << QString("(Run) Added uid:  ") << QString(uid);
			data->uids.push_back(uid);
			qDebug() << QString("(Run) Check uids: ") << QString(data->uids[data->uids.size()-1]);
			data->patientDirectoryPath[uid] = ui->patientTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toStdString();

			for (int j = 0; j < ui->patientTree->topLevelItem(i)->childCount(); j++)
			{
				if (ui->patientTree->topLevelItem(i)->child(j)->checkState(0) == Qt::Checked) {
					if (ui->patientTree->topLevelItem(i)->child(j)->text(0).toStdString() == "<Mask>") {
						data->maskPath[uid] = ui->patientTree->topLevelItem(i)->child(j)->data(0, Qt::UserRole).toString().toStdString();
					}
					else {
						data->imagesPaths[uid].push_back(ui->patientTree->topLevelItem(i)->child(j)->data(0, Qt::UserRole).toString().toStdString());
					}
				}
			}

			if (data->imagesPaths.find(uid) == data->imagesPaths.end() || data->maskPath.find(uid) == data->maskPath.end()) {
				qDebug() << QString("(Run) Images or mask missing so the uid was dismissed");
				data->uids.pop_back();
			}
		}
	}

	qDebug() << QString("Trying to run");
	m_Scheduler.SetData(data);
	m_Scheduler.Start();
}

void MainWindow::SchedulerResultReady(long uid)
{
	qDebug() << QString("SchedulerResultReady called for uid: ") << QString(uid);

	if (subjectByUid[uid])
	{
		subjectByUid[uid]->setText(0, subjectByUid[uid]->text(0).append(QString(" [FINISHED]")));

		QTreeWidgetItem *imageItem = new QTreeWidgetItem(subjectByUid[uid]);

		QString file = subjectByUid[uid]->data(0, Qt::UserRole).toString() + QString("/MPIP_output/labels_res.nii.gz");

		imageItem->setText(0, QString("<Segmentation>"));
		imageItem->setCheckState(0, Qt::Checked);

		imageItem->setData(0, Qt::UserRole, file);          // path to the image
		imageItem->setData(0, Qt::UserRole + 1, QString("<Segmentation>"));
	}
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
	
	QString patientName = QString::fromStdString(
		directoryPath.toStdString().substr(
			directoryPath.toStdString().find_last_of("/\\") + 1
		)
	);

	//patientToAdd->setText(0, patientName);
	patientToAdd->setCheckState(0, Qt::Checked);
	patientToAdd->setData(0, Qt::UserRole, directoryPath);
	patientToAdd->setData(0, Qt::UserRole + 1, uidNextToGive);
	subjectByUid[uidNextToGive] = patientToAdd;
	uidNextToGive++;

	QProgressBar *progressBar = new QProgressBar();
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

	QWidget *labelAndProgress = new QWidget();
	labelAndProgress->setStyleSheet("background-color: rgba(0,0,0,0)");
	labelAndProgress->setMaximumHeight(35);
	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addWidget(label, Qt::AlignLeft);
	hLayout->addWidget(progressBar, Qt::AlignRight);
	labelAndProgress->setLayout(hLayout);
	
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

		imageItem->setData(0, Qt::UserRole, file);          // path to the image
		imageItem->setData(0, Qt::UserRole + 1, shortName); // short name
	}

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

void MainWindow::SwitchSubjectAndImage(size_t subjectPos, size_t imagePos)
{
	Load(
		ui->patientTree->topLevelItem(subjectPos)->child(imagePos)->data(0, Qt::UserRole).toString()
	);
}
