#include "GeodesicTrainingGUI.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QString>
#include <QFileInfo>
#include <QRegExp>
#include <QDebug>
#include <QtConcurrent>

#include <mitkIOUtil.h>

#include "DataViewBase.h"
#include "SchedulerBase.h"
#include "DefaultScheduler.h"
#include "WarningManager.h"
#include "MitkSegmentationTool.h"
#include "CustomMitkDataStorage.h"
#include "AlgorithmModuleBase.h"
#include "InfoLabel.h"
#include "GeodesicTrainingModule.h"
#include "GeodesicTrainingWarningImageSize.h"
#include "GeodesicTrainingWarningGUI.h"

#include "ui_GeodesicTrainingGUI.h"

GeodesicTrainingGUI::GeodesicTrainingGUI(QWidget *parent) :
  GuiModuleBase(parent),
  ui(new Ui::GeodesicTrainingGUI),
  m_DataStorage(CustomMitkDataStorage::GetInstance()),
  m_Scheduler(DefaultScheduler::GetInstance()),
  m_MitkSegmentationTool(new MitkSegmentationTool(nullptr))
{
  ui->setupUi(this);
  m_MitkSegmentationTool->SetSpecialRoleOfInterest("Seeds");
  m_MitkSegmentationTool->SetAllowMultiple(false);
  m_MitkSegmentationTool->findChild<QPushButton*>("createMaskPushBtn")->setText(
    "Create seeds image"
  );
  m_MitkSegmentationTool->findChild<InfoLabel*>("infoLabel")->hide();
  ui->infoLabelGeodesic->setText(
    QString("<b>1.</b> Create seeds image.<br>") +
    QString("<b>2.</b> Draw with at least two colors.<br>") +
    QString("<b>3.</b> Click run and wait.<br>") +
    QString("<b>4.</b> If the output segmentation") +
    QString(" contains mistakes, draw over them on the seeds image") +
    QString(" with the correct color and run again.")
  );
  GuiModuleBase::PlaceWidgetInWidget(m_MitkSegmentationTool, ui->mitkDrawingToolContainer);

  ui->geodesicWarnings->hide();

  connect(ui->pushButtonRun, SIGNAL(clicked()), this, SLOT(OnRunClicked()));
  ui->pushButtonRun->hide();

  connect(m_Scheduler, SIGNAL(JobFinished(AlgorithmModuleBase*)), 
    this, SLOT(OnSchedulerJobFinished(AlgorithmModuleBase*))
  );
}

GeodesicTrainingGUI::~GeodesicTrainingGUI()
{
  delete ui;
}

void GeodesicTrainingGUI::SetDataManager(DataManager* dataManager)
{
  GuiModuleBase::SetDataManager(dataManager);
  m_MitkSegmentationTool->SetDataManager(dataManager);
}

void GeodesicTrainingGUI::SetDataView(DataViewBase* dataView)
{
  m_DataView = dataView;
  connect(m_DataView, SIGNAL(SelectedSubjectChanged(long)), 
    this, SLOT(SelectedSubjectChangedHandler(long))
  );
  connect(m_DataView, SIGNAL(DataAddedForSelectedSubject(long)), 
    this, SLOT(DataAddedForSelectedSubjectHandler(long))
  );
  connect(m_DataView, SIGNAL(DataRemovedFromSelectedSubject(long)), 
    this, SLOT(DataRemovedFromSelectedSubjectHandler(long))
  );

  m_MitkSegmentationTool->SetDataView(m_DataView);
}

void GeodesicTrainingGUI::SetScheduler(SchedulerBase* scheduler)
{
  m_Scheduler = scheduler;
}

void GeodesicTrainingGUI::SetEnabled(bool enabled)
{
  m_MitkSegmentationTool->SetEnabled(enabled);
  this->SetUpWarnings();
}

void GeodesicTrainingGUI::AllowCreatingSeeds(bool allow)
{
  qDebug() << "GeodesicTrainingGUI::AllowCreatingSeeds" << ((allow)?"yes":"no");

  auto seeds = this->GetDataManager()->GetAllDataIdsOfSubjectWithSpecialRole(
    m_DataView->GetCurrentSubjectID(), "Seeds"
  );

  bool noOtherSeeds = (seeds.size() == 0);

  if (allow && noOtherSeeds)
  {
    this->findChild<QFrame*>("frameCreate")->show();
  }

  if (!allow)
  {
    for (const long& seed : seeds) { this->GetDataManager()->RemoveData(seed); }
    this->findChild<QFrame*>("frameCreate")->hide();
  }
}

void GeodesicTrainingGUI::SelectedSubjectChangedHandler(long uid)
{
  long seed  = -1;
  auto seeds = this->GetDataManager()->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Seeds");
  if (seeds.size() > 0) 
  { 
    seed = seeds[seeds.size()-1]; 
    ui->pushButtonRun->show();
  }
  else {
    ui->pushButtonRun->hide();
  }
  m_MitkSegmentationTool->ChangeFocusImage(seed);
}

void GeodesicTrainingGUI::DataAddedForSelectedSubjectHandler(long iid)
{
  if (this->GetDataManager()->GetDataSpecialRole(iid) == "Seeds")
  {
    m_MitkSegmentationTool->ChangeFocusImage(iid);
    ui->pushButtonRun->show();
  }
}

void GeodesicTrainingGUI::DataRemovedFromSelectedSubjectHandler(long iid)
{
  long uid   = m_DataView->GetCurrentSubjectID();
  long seed  = -1;
  auto seeds = this->GetDataManager()->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Seeds");
  if (seeds.size() > 0) { 
    seed = seeds[seeds.size()-1]; 
    ui->pushButtonRun->show();
  }
  else {
    ui->pushButtonRun->hide();
  }
  m_MitkSegmentationTool->ChangeFocusImage(seed);
}

void GeodesicTrainingGUI::OnRunClicked()
{
  qDebug() << "GeodesicTrainingGUI::OnRunClicked";
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
  m_SubjectsThatAreRunning.insert(uid);

  // Remove all previous segmentations (if they exist)
  for (const long& iid : m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Segmentation"))
  {
    if (m_DataManager->GetDataName(iid) == "<Segmentation>   ")
    {
      m_DataManager->RemoveData(iid);
    }
  }

	AlgorithmModuleBase* algorithm = new GeodesicTrainingModule();

  algorithm->SetDataManager(m_DataManager);
  algorithm->SetUid(uid);
  algorithm->SetDataView(m_DataView);
  algorithm->SetAppName(m_AppName);
  algorithm->SetAppNameShort(m_AppNameShort);

  // connect(algorithm, SIGNAL(ProgressUpdateUI(long, QString, int)), 
  //   m_DataView, SLOT(UpdateProgressHandler(long, QString, int))
  // );
  connect(algorithm, SIGNAL(AlgorithmFinished(AlgorithmModuleBase*)),
    this, SLOT(OnAlgorithmFinished(AlgorithmModuleBase*))
  );
  connect(algorithm, SIGNAL(AlgorithmFinishedWithError(AlgorithmModuleBase*, QString)),
    this, SLOT(OnAlgorithmFinishedWithError(AlgorithmModuleBase*, QString))
  );

  ul.unlock();
  CustomMitkDataStorage::GetInstance()->WriteChangesToFileForAllImagesOfCurrentSubject();
  m_Scheduler->QueueAlgorithm(algorithm);
}

void GeodesicTrainingGUI::OnSchedulerJobFinished(AlgorithmModuleBase* algorithmModuleBase)
{
  if (algorithmModuleBase->GetAlgorithmNameShort() == "GeodesicTraining")
  {
    m_SubjectsThatAreRunning.erase(algorithmModuleBase->GetUid());
    delete algorithmModuleBase;
  }
}

void GeodesicTrainingGUI::OnAlgorithmFinished(AlgorithmModuleBase* algorithmModuleBase)
{
  if (algorithmModuleBase->GetAlgorithmNameShort() == "GeodesicTraining")
  {

  }
}

void GeodesicTrainingGUI::OnAlgorithmFinishedWithError(AlgorithmModuleBase* algorithmModuleBase, 
  QString errorMessage)
{
  QMessageBox::warning(
		this,
		algorithmModuleBase->GetAlgorithmName(),
		errorMessage	
	);
}

void GeodesicTrainingGUI::SetUpWarnings()
{
  // --- Create Warning Manager ---
  WarningManager* warningManager = new WarningManager(this);
  
  // --- Create Warning GUI ---
  auto warningGUI = new GeodesicTrainingWarningGUI();
  warningGUI->SetWidgetContainer(ui->geodesicWarnings);

  // --- Connect warningManager with warningGUI ---
  connect(warningManager, SIGNAL(NewErrorMessage(QString)),
    warningGUI, SLOT(OnNewErrorMessage(QString))
  );
  connect(warningManager, SIGNAL(ErrorMessageWasRemoved(QString)),
    warningGUI, SLOT(OnErrorMessageWasRemoved(QString))
  );
  connect(warningManager, SIGNAL(NewWarning(QString)),
    warningGUI, SLOT(OnNewWarning(QString))
  );
  connect(warningManager, SIGNAL(WarningWasRemoved(QString)),
    warningGUI, SLOT(OnWarningWasRemoved(QString))
  );
  connect(warningManager, SIGNAL(NewWarningFunctionAdded(WarningFunctionBase*)),
    warningGUI, SLOT(OnNewWarningFunctionAdded(WarningFunctionBase*))
  );
  connect(warningGUI, SIGNAL(CreatingSeedsAllowance(bool)),
    this, SLOT(AllowCreatingSeeds(bool))
  );

  // --- Add Warning Functions ---
  qDebug() << m_DataView->GetCurrentDataID();

  auto wImageSize = new GeodesicTrainingWarningImageSize(warningManager);
  warningManager->RegisterWarningFunction(wImageSize);
  wImageSize->SetDataManager(this->GetDataManager());
  wImageSize->SetDataView(m_DataView);
  
  // Add more warnings here if necessary
}