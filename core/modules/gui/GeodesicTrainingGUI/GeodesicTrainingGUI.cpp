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
#include "MitkImageViewer.h"
#include "MitkDrawingTool.h"

#include "AlgorithmModuleBase.h"

#include "ui_GeodesicTrainingGUI.h"

GeodesicTrainingGUI::GeodesicTrainingGUI(mitk::DataStorage *datastorage, QWidget *parent) :
  GuiModuleBase(parent),
  ui(new Ui::GeodesicTrainingGUI),
  m_DataStorage(datastorage),
  m_MitkDrawingTool(new MitkDrawingTool(datastorage, this))
{
  ui->setupUi(this);
  GuiModuleBase::PlaceWidgetInWidget(m_MitkDrawingTool, ui->mitkDrawingToolContainer);

  connect(ui->pushButtonRun, SIGNAL(clicked), this, SLOT(OnRunClicked()));
}

GeodesicTrainingGUI::~GeodesicTrainingGUI()
{
  delete ui;
}

void GeodesicTrainingGUI::SetMitkImageViewer(MitkImageViewer* mitkImageViewer)
{
  m_MitkImageViewer = mitkImageViewer;
  m_MitkDrawingTool->SetMitkImageViewer(mitkImageViewer);
}

void GeodesicTrainingGUI::SetDataManager(DataManager* dataManager)
{
  m_DataManager = dataManager;
  m_MitkDrawingTool->SetDataManager(dataManager);
}

void GeodesicTrainingGUI::SetDataView(DataViewBase* dataViewBase)
{
  m_DataView = dataViewBase;
  m_MitkDrawingTool->SetDataView(dataViewBase);
}

void GeodesicTrainingGUI::OnRunClicked()
{
  // qDebug() << "GeodesicTrainingGUI::OnRunClicked";
	// long uid = m_DataView->GetCurrentSubjectID(); // For convenience

	// if (uid == -1)
	// {
	// 	QMessageBox::information(
	// 		this,
	// 		tr("No subject selected"),
	// 		tr("Please select a subject.")
	// 	);
	// 	return;
	// }

  // if (m_SubjectsThatAreRunning.count(uid) != 0)
  // {
  //   QMessageBox::warning(this,
  //     "Please wait",
  //     "Algorithm is already running for this subject"	
  //   );
  //   return;
  // }

  // std::unique_lock<std::mutex> ul(*m_DataManager->GetSubjectEditMutexPointer(uid));
	
  // qDebug() << QString("(Run) uid:  ") << QString::number(uid);

  // auto iids = m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(
  //   uid, "Mask"
  // );

  // long maskNrrd = -1;

  // for (const long& iid : iids)
  // {
  //   if (m_DataManager->GetDataPath(iid).endsWith(".nrrd", Qt::CaseSensitive))
  //   {
  //     maskNrrd = iid;
  //     break;
  //   }
  // }

  // if (maskNrrd != -1)
  // {
  //   qobject_cast<MitkImageViewer*>(m_ImageViewer)->SaveImageToFile(maskNrrd, false);
  // }

  // // Remove all previous masks (if they exist)
  // for (const long& iid : iids)
  // {
  //   if (iid != maskNrrd)
  //   {
  //     m_DataManager->RemoveData(iid);
  //   }
  // }  

  // // Remove all previous segmentations (if they exist)
  // for (const long& iid : m_DataManager->GetAllDataIdsOfSubjectWithSpecialRole(uid, "Segmentation"))
  // {
  //   if (m_DataManager->GetDataName(iid) == "<Segmentation>")
  //   {
  //     m_DataManager->RemoveData(iid);
  //   }
  // }

	// AlgorithmModuleBase* algorithm = new GeodesicTrainingModule();

  // m_SubjectsThatAreRunning.insert(uid);

  // algorithm->SetDataManager(m_DataManager);
  // algorithm->SetUid(uid);
  // algorithm->SetAppName(m_AppName);
  // algorithm->SetAppNameShort(m_AppNameShort);

  // connect(algorithm, SIGNAL(ProgressUpdateUI(long, QString, int)), 
  //   m_DataView, SLOT(UpdateProgressHandler(long, QString, int))
  // );
  // connect(algorithm, SIGNAL(AlgorithmFinished(AlgorithmModuleBase*)),
  //   this, SLOT(OnAlgorithmFinished(AlgorithmModuleBase*))
  // );
  // connect(algorithm, SIGNAL(AlgorithmFinishedWithError(AlgorithmModuleBase*, QString)),
  //   this, SLOT(OnAlgorithmFinishedWithError(AlgorithmModuleBase*, QString))
  // );

  // ul.unlock();
  // m_Scheduler->QueueAlgorithm(algorithm);
}