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
#include "MitkSegmentationTool.h"
#include "CustomMitkDataStorage.h"
#include "AlgorithmModuleBase.h"
#include "InfoLabel.h"
#include "GeodesicTrainingModule.h"

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
  m_MitkSegmentationTool->findChild<InfoLabel*>("infoLabel")->setText(
    QString("<b>1.</b> Create seeds image.<br>") +
    QString("<b>2.</b> Draw with at least two colors.<br>") +
    QString("<b>3.</b> Click run and wait.<br>") +
    QString("<b>4.</b> If the output segmentation") +
    QString(" contains mistakes, draw over them on the seeds image") +
    QString(" with the correct color and run again.")
  );
  GuiModuleBase::PlaceWidgetInWidget(m_MitkSegmentationTool, ui->mitkDrawingToolContainer);

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
  m_DataManager = dataManager;
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