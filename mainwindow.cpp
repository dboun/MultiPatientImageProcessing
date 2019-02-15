#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  m_VtkViewer = new VtkViewer();

  ui->setupUi(this);

  // Replace viewerContainer with viewer of choice
  QGridLayout *layout = new QGridLayout(ui->viewerContainer);
  layout->addWidget(m_VtkViewer, 0, 0);

  ui->actionAdd_image_for_selected_subject->setVisible(false);
  ui->actionAdd_image_for_new_subject->setVisible(false);
  ui->actionAdd_multiple_subjects->setVisible(false);
  ui->pushButtonConfigure->setVisible(false);
  ui->actionOpen_Dicom->setVisible(false);

  //dicomReader = new DicomReader();
  //dcmdisplayWidget = new DicomMetaDataDisplayWidget();
  //this->SetupWidgets();

  setAcceptDrops(true); // For drag and drop
  m_Scheduler.connect(&m_Scheduler, SIGNAL(jobFinished(long)), this, SLOT(SchedulerResultReady(long)));
  m_Scheduler.connect(&m_Scheduler, SIGNAL(updateProgress(long, int)), this, SLOT(UpdateProgress(long, int)));

  QStringList columnNames = QStringList() << "Select subjects" << "Progress";

  ui->patientTree->setHeaderLabels(columnNames);
  ui->patientTree->setColumnCount(2);

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
  // Not implemented yet
  QStringList apps;

//#ifdef BUILD_GEODESIC_TRAINING
  //apps << "Geodesic Training Segmentation";
//#endif // BUILD_GEODESIC_TRAINING

}

void MainWindow::OnTreeWidgetClicked(QTreeWidgetItem *item, int column)
{
  qDebug() << QString("Clicked tree item.");
  
  qDebug() << QString("Value of parent: ") << 
    ((item->checkState(0) == Qt::Checked) ? QString("Checked") : QString("Unchecked"));
  
  // If it's a parent item
  for (int i = 0; i < item->childCount(); i++) {
    item->child(i)->setCheckState(0, item->checkState(0));
    qDebug() << QString("Changed the value of a child");
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
  this->ui->stackedWidget->setCurrentIndex(0);
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
    if (ui->patientTree->topLevelItem(i) && ui->patientTree->topLevelItem(i)->checkState(0) == Qt::Checked)
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
    //subjectByUid[uid]->setText(0, subjectByUid[uid]->text(0).append(QString(" [FINISHED]")));

    QTreeWidgetItem *imageItem = new QTreeWidgetItem(subjectByUid[uid]);

    QString file = subjectByUid[uid]->data(0, Qt::UserRole).toString() + QString("/MPIP_output/labels_res.nii.gz");

    imageItem->setText(0, QString("<Segmentation>"));
    imageItem->setCheckState(0, Qt::Checked);

    imageItem->setData(0, Qt::UserRole, file);          // path to the image
    imageItem->setData(0, Qt::UserRole + 1, QString("<Segmentation>"));
  }
}

void MainWindow::UpdateProgress(long uid, int progress)
{
  QProgressBar *progressBar = subjectByUid[uid]->treeWidget()->findChild<QProgressBar*>(QString("ProgressBar") + QString(uid));
  progressBar->setVisible(true);
  progressBar->setValue(progress);
}

void MainWindow::Load(QString filepath)
{
  m_VtkViewer->Display(filepath);
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
  labelAndProgress->setMaximumHeight(35);
  QHBoxLayout *hLayout = new QHBoxLayout();
  //hLayout->addWidget(label, Qt::AlignLeft);
  hLayout->addWidget(progressBar, Qt::AlignRight);
  labelAndProgress->setLayout(hLayout);
  labelAndProgress->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  
  patientToAdd->setText(0, patientName);
  ui->patientTree->setItemWidget(patientToAdd, 1, labelAndProgress);

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

void MainWindow::SwitchSubjectAndImage(size_t subjectPos, size_t imagePos)
{
  Load(
    ui->patientTree->topLevelItem(subjectPos)->child(imagePos)->data(0, Qt::UserRole).toString()
  );
}