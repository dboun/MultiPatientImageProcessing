#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QtConcurrent>

#include "MitkDrawingTool.h"
#include "ui_MitkDrawingTool.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"
#include "QmitkNewSegmentationDialog.h"
#include <mitkIOUtil.h>
#include "QmitkToolGUI.h"

#include "DataManager.h"

MitkDrawingTool::MitkDrawingTool(mitk::DataStorage *datastorage, QWidget *parent) :
    GuiModuleBase(parent),
    ui(new Ui::MitkDrawingTool),
    m_DataStorage(datastorage),
    m_LastToolGUI(nullptr)
{
    ui->setupUi(this);

    ui->infoLabel->setText(
      QString("<b>1.</b> Create Mask.<br>") +
      QString("<b>2.</b> Draw with at least two colors.<br>") +
      QString("<b>3.</b> Click run and wait.<br>") +
      QString("<b>4.</b> If the output segmentation") +
      QString(" contains mistakes,") +
      QString(" draw over them on the mask") +
      QString(" with the correct color") +
      QString(" and run again.")
    );

    ui->newLabelPushBtn->hide();
    ui->labelSetWidget->hide();
    ui->createMaskPushBtn->show();

    this->m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    assert(m_ToolManager);

    m_ToolManager->SetDataStorage(*m_DataStorage);
    m_ToolManager->InitializeTools();

    ui->labelSetWidget->SetDataStorage(m_DataStorage);
    ui->labelSetWidget->SetOrganColors(mitk::OrganNamesHandling::GetDefaultOrganColorString());
    ui->labelSetWidget->findChild<ctkSearchBox*>("m_LabelSearchBox")->hide();
    //ui->labelSetWidget->hide();

    ui->toolSelectionBox->SetToolManager(*m_ToolManager);
    ui->toolSelectionBox->setEnabled(true);
    ui->toolSelectionBox->SetGenerateAccelerators(true);
    ui->toolSelectionBox->SetDisplayedToolGroups(tr("Add Subtract Correction Paint Erase").toStdString());
    ui->toolSelectionBox->SetLayoutColumns(3);
    ui->toolSelectionBox->SetEnabledMode(QmitkToolSelectionBox::AlwaysEnabled);

    connect(ui->newLabelPushBtn, SIGNAL(clicked()), this, SLOT(OnCreateNewLabel()));
    connect(ui->createMaskPushBtn, SIGNAL(clicked()), this, SLOT(OnCreateNewMask()));
    connect(ui->ConfirmSegBtn, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()));
    connect(ui->toolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)));
    
    ui->toolGUIArea->setVisible(false);
    ui->toolSelectionBox->setVisible(false);

    // Unused
    ui->ConfirmSegBtn->setVisible(false);

    m_ProgressDialogWatcher = new QFutureWatcher<void>(this);
    connect(m_ProgressDialogWatcher, SIGNAL(finished()),
      this, SLOT(OnCreateEmptyMaskBackgroundFinished())
    );
}

MitkDrawingTool::~MitkDrawingTool()
{
  if (m_LoadedMaskNode)
  {
    this->OnDataAboutToGetRemoved(
      QString(m_LoadedMaskNode->GetName().c_str()).toLong()
    );
  }

  delete ui;
}

void MitkDrawingTool::SetMitkImageViewer(MitkImageViewer* mitkImageViewer)
{
  // Signals from MitkImageViewer
  connect(mitkImageViewer, SIGNAL(MitkLoadedNewMask(mitk::DataNode::Pointer)),
    this, SLOT(OnMitkLoadedNewMask(mitk::DataNode::Pointer))
  );
  connect(mitkImageViewer, SIGNAL(MitkNodeAboutToBeDeleted(long)),
    this, SLOT(OnDataAboutToGetRemoved(long))
  );

  // Signals to MitkImageViewer
  connect(this, SIGNAL(MitkDrawingToolSaveImageToFile(long, bool)),
    mitkImageViewer, SLOT(SaveImageToFile(long, bool))
  );

}

void MitkDrawingTool::OnDataAboutToGetRemoved(long iid)
{
  qDebug() << "MitkDrawingTool::OnDataAboutToGetRemoved";

  if (!m_MaskLoadedForThisSubject || !m_LoadedMaskNode || iid == -1)
  {
    return;
  }

  long loadedMaskIid = QString(m_LoadedMaskNode->GetName().c_str()).toLong();

  if (loadedMaskIid == iid)
  {
    ui->newLabelPushBtn->hide();
    ui->labelSetWidget->hide();
    ui->createMaskPushBtn->show();

    emit MitkDrawingToolSaveImageToFile(loadedMaskIid, false);

    m_MaskLoadedForThisSubject = false;
    
    ui->toolGUIArea->setVisible(false);
    ui->toolSelectionBox->setVisible(false);

    mitk::LabelSetImage::Pointer emptyImage = mitk::LabelSetImage::New();
    emptyImage->Initialize(dynamic_cast<mitk::Image*>(m_LoadedMaskNode->GetData()));
    mitk::DataNode::Pointer emptyNode = mitk::DataNode::New();
    emptyNode->SetData(emptyImage);
    //m_DataStorage->Add(emptyNode);

    m_ToolManager->SetWorkingData(emptyNode);
    m_ToolManager->SetReferenceData(emptyNode); 

    //ui->labelSetWidget->UpdateAllTableWidgetItems();
    ui->labelSetWidget->ResetAllTableWidgetItems();
    
    m_LoadedMaskNode = nullptr;
  }

  qDebug() << "MitkDrawingTool::OnDataAboutToGetRemovedFinished";
}

void MitkDrawingTool::OnMitkLoadedNewMask(mitk::DataNode::Pointer dataNode)
{
  ui->newLabelPushBtn->show();
  ui->labelSetWidget->show();
  ui->createMaskPushBtn->hide();

  qDebug() << "MitkDrawingTool::OnMitkLoadedNewMask";
  m_MaskLoadedForThisSubject = true;

  // Connect the mask to the tool
  m_LoadedMaskNode = dataNode;
  
  this->m_ToolManager->SetWorkingData(m_LoadedMaskNode);
  this->m_ToolManager->SetReferenceData(m_LoadedMaskNode);
  
  if (ui->labelSetWidget->isHidden()) {
    ui->labelSetWidget->show();
  }
  ui->labelSetWidget->ResetAllTableWidgetItems();

  ui->toolGUIArea->setVisible(true);
  ui->toolSelectionBox->setVisible(true);

  mitk::RenderingManager::GetInstance()->InitializeViews(
    m_LoadedMaskNode->GetData()->GetTimeGeometry(), 
    mitk::RenderingManager::REQUEST_UPDATE_ALL, true
  );

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(m_LoadedMaskNode->GetData());
  if (workingImage->GetTotalNumberOfLabels() == 1) // 1 means none
  {
    this->OnCreateNewLabel();
  }
  qDebug() << "Number of labels in loaded mask" << workingImage->GetTotalNumberOfLabels();

  // // Check if this was triggered by pressing the new label button
  // if (m_WaitingOnLabelsImageCreation)
  // {
	//   this->OnCreateNewLabel();
  //   m_WaitingOnLabelsImageCreation = false;
  // }
}

void MitkDrawingTool::SetDataManager(DataManager* dataManager)
{
  m_DataManager = dataManager;

  connect(m_DataManager, SIGNAL(DataAboutToGetRemoved(long)),
    this, SLOT(OnDataAboutToGetRemoved(long))
  );
}

void MitkDrawingTool::SetDataView(DataViewBase* dataViewBase)
{
  connect(dataViewBase, SIGNAL(DataRequestedAsMask(long)),
    this, SLOT(SetMaskFromNiftiData(long))
  );
}

// void MitkDrawingTool::OnEnableSegmentation()
// {
//   m_LabelsImageName.clear();

//   QString path = m_DataManager->GetDataPath(m_CurrentData);
//   QFileInfo f(path);
//   mitk::DataNode::Pointer referenceNode = this->m_DataStorage->GetNamedNode(f.baseName().toStdString().c_str());

//   if (!referenceNode)
//   {
//     QMessageBox::information(
//       this, "New Segmentation Session", "Please load and select a patient image before starting some action.");
//     return;
//   }

//   m_ToolManager->ActivateTool(-1);

//   mitk::Image* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
//   assert(referenceImage);

//   m_LabelsImageName = QString::fromStdString(m_DataManager->GetSubjectName(
// 	  m_DataManager->GetSubjectIdFromDataId(m_CurrentData)
//   ).toStdString());
//   m_LabelsImageName.append("-mask");

//   bool ok = false;
//   m_LabelsImageName = QInputDialog::getText(this, "New Segmentation Session", "New name:", QLineEdit::Normal, m_LabelsImageName, &ok);

//   if (!ok)
//   {
//     return;
//   }

//   mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();
//   try
//   {
//     workingImage->Initialize(referenceImage);
//   }
//   catch (mitk::Exception& e)
//   {
//     MITK_ERROR << "Exception caught: " << e.GetDescription();
//     QMessageBox::information(this, "New Segmentation Session", "Could not create a new segmentation session.\n");
//     return;
//   }

//   mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
//   workingNode->SetData(workingImage);
//   workingNode->SetName(m_LabelsImageName.toStdString());

//   if (!this->m_DataStorage->Exists(workingNode))
//   {
//     this->m_DataStorage->Add(workingNode, referenceNode);
//   }

//   this->m_ToolManager->SetWorkingData(workingNode);
//   this->m_ToolManager->SetReferenceData(referenceNode);

//   OnCreateNewLabel();
// }

// void MitkDrawingTool::CreateNewSegmentation()
// {
//   // Create empty segmentation working image
//   mitk::DataNode::Pointer workingImageNode = mitk::DataNode::New();
//   mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingImageNode->GetData());
//   const std::string organName = "test";
//   mitk::Color color; // actually it dosn't matter which color we are using
//   color.SetRed(1);   // but CreateEmptySegmentationNode expects a color parameter
//   color.SetGreen(0);
//   color.SetBlue(0);
//   mitk::Tool* firstTool = m_ToolManager->GetToolById(0);
//   workingImageNode = firstTool->CreateEmptySegmentationNode(workingImage, organName, color);
//   this->m_DataStorage->Add(workingImageNode);
//   if (workingImageNode.IsNotNull())
//     this->m_ToolManager->SetWorkingData(workingImageNode);
//   mitk::DataNode::Pointer origNode = this->m_DataStorage->GetNamedNode("LoadedData");
//   if (origNode.IsNotNull())
//     this->m_ToolManager->SetReferenceData(origNode);
//   mitk::RenderingManager::GetInstance()->InitializeViews(workingImageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
// }

// void MitkDrawingTool::OnDisableSegmentation()
// {
//   if (m_LastToolGUI)
//   {
//     ui->toolGUIArea->layout()->removeWidget(m_LastToolGUI);
//     m_LastToolGUI->setParent(nullptr);
//     delete m_LastToolGUI; // will hopefully notify parent and layouts
//     m_LastToolGUI = nullptr;

//     QLayout *layout = ui->toolGUIArea->layout();
//     if (layout)
//     {
//       layout->activate();
//     }
//   }
// }

void MitkDrawingTool::OnCreateNewLabel()
{
  if (!m_MaskLoadedForThisSubject)
  {
    return;
  }

  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    QMessageBox::information(this, 
      "Drawing Tool", 
      "Please load and select a patient image before starting some action."
    );
    return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (!workingImage)
  {
    QMessageBox::information(
      this, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog(this);
  dialog->SetSuggestionList(mitk::OrganNamesHandling::GetDefaultOrganColorString());
  dialog->setWindowTitle("New Label");
  dialog->setPrompt("Choose label color and name (optional).");

  int dialogReturnValue = dialog->exec();
  if (dialogReturnValue == QDialog::Rejected)
  {
    return;
  }

  QString labelsImageName = dialog->GetSegmentationName();
  if (labelsImageName.isEmpty())
  {
    labelsImageName = "Unnamed";
  }
  workingImage->GetActiveLabelSet()->AddLabel(labelsImageName.toStdString(), dialog->GetColor());

  if (ui->labelSetWidget->isHidden())
    ui->labelSetWidget->show();
  ui->labelSetWidget->ResetAllTableWidgetItems();

  mitk::RenderingManager::GetInstance()->InitializeViews(workingNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
}

void MitkDrawingTool::OnCreateNewMask()
{
    // Find the first node, if there is one
    long referenceIid = -1;
    mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) 
    {  
      QString nodeName = QString(it.Value()->GetName().c_str());
      if (QRegExp("\\d*").exactMatch(nodeName)) { // a digit (\d), zero or more times (*)
        referenceIid = nodeName.toLong();
        break;
      }
    }
    
    if (referenceIid != -1)
    {
      ui->createMaskPushBtn->hide();
      //m_WaitingOnLabelsImageCreation = true;
      emit MitkDrawingToolCreateEmptyMask(referenceIid);
      
      m_ProgressDialog = new QProgressDialog(this);
      m_ProgressDialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | 
        Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint
      );
      m_ProgressDialog->setWindowTitle("Please wait");
      m_ProgressDialog->setCancelButton(nullptr);
      //Set progress dialog in undeterminate state: 
      m_ProgressDialog->setMaximum( 0 );
      m_ProgressDialog->setMinimum( 0 );
      m_ProgressDialog->show();


      QFuture<void> future = QtConcurrent::run(
        this, &MitkDrawingTool::CreateEmptyMask, 
        referenceIid
      );
      m_ProgressDialogWatcher->setFuture(future);

    }
    else {
      QMessageBox::information(this, 
        "Drawing Tool", 
        "Please load a subject before starting some action."
      );
    }
}

void MitkDrawingTool::SetMaskFromNiftiData(long iid)
{
  mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>(
      this->GetDataManager()->GetDataPath(iid).toStdString()
  );
  mitk::LabelSetImage::Pointer maskImage = mitk::LabelSetImage::New();
	
	// Copy the data from input image
	maskImage->InitializeByLabeledImage(inputImage);
  

  long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);

  // Create the directory to save if it doesn't exist
  QString directoryName = this->GetDataManager()->GetSubjectPath(uid)
      + QString("/") + m_AppNameShort + QString("/")
      + m_AppNameShort + "_" + "Mask";

  if (!QDir(directoryName).exists())
  {
      QDir().mkpath(directoryName);
  }

  QString nifti = directoryName + QString("/mask.nii.gz");
  QString nrrd  = directoryName + QString("/mask.nrrd");

  this->GetDataManager()->RemoveData(iid);
  
  // Remove previous masks
  auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(uid);

  for (const long& tIid : iids)
  {
      QString tPath = this->GetDataManager()->GetDataPath(tIid);

      if (tPath == nifti || tPath == nrrd)
      {
          this->GetDataManager()->RemoveData(tIid);
      }
  }

  // Save
  mitk::IOUtil::Save(
      maskImage,
      nifti.toStdString()
  );

  mitk::IOUtil::Save(
      maskImage,
      nrrd.toStdString()
  );

  // Update DataManager
  this->GetDataManager()->AddDataToSubject(
      uid, nifti, "Mask"
  );

  this->GetDataManager()->AddDataToSubject(
      uid, nrrd, "Mask"
  );
}

void MitkDrawingTool::OnConfirmSegmentation()
{
  // QString pathToSave = QDir::currentPath();
  // if (m_DataManager->GetSubjectIdFromDataId(m_CurrentData) != -1)
  // {
  //   pathToSave = m_DataManager->GetSubjectPath(m_DataManager->GetSubjectIdFromDataId(m_CurrentData));
  // }

  // QString filename = QFileDialog::getSaveFileName(this, tr("Save Segmentation Mask"),
  //   pathToSave, tr("Images (*.nrrd)"));
  
  // // if (!filename.endsWith(".nii.gz"))
  // // {
	//  //  filename = filename + ".nii.gz";
  // // }

  // if (!filename.isEmpty())
  // {
  //   mitk::DataNode::Pointer segData = this->m_DataStorage->GetNamedNode(m_LabelsImageName.toStdString());
  //   mitk::IOUtil::Save(segData->GetData(), filename.toStdString());

  //   //remove after saving
  //   m_DataStorage->Remove(m_DataStorage->GetNamedNode(m_LabelsImageName.toStdString()));
    
  //   if (m_DataManager->GetSubjectIdFromDataId(m_CurrentData) != -1)
  //   {
	// 	m_DataManager->AddDataToSubject(
	// 		m_DataManager->GetSubjectIdFromDataId(m_CurrentData),
	// 		filename,
	// 		"Mask"
	// 	);
  //   }
  // }

  // //re-render here
  // mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // //hide ourself
  // this->hide();
}

void MitkDrawingTool::OnManualTool2DSelected(int id)
{
  qDebug() << "MitkDrawingTool::OnManualTool2DSelected";

  if (!m_MaskLoadedForThisSubject)
  {
    return;
  }

  qDebug() << "MitkDrawingTool::OnManualTool2DSelected (mask is loaded)";

  if (id >= 0)
  {
    std::string text = m_ToolManager->GetToolById(id)->GetName();
    
    if (text == "Paint")
    {
      //this->OnDisableSegmentation();
        this->RemoveExistingToolGui();

      mitk::Tool *tool = m_ToolManager->GetActiveTool();

      itk::Object::Pointer possibleGUI = tool->GetGUI("Qmitk", "GUI").GetPointer(); 
      QmitkToolGUI *gui = dynamic_cast<QmitkToolGUI *>(possibleGUI.GetPointer());
      m_LastToolGUI = gui;
      if (gui)
      {
        gui->SetTool(tool);

        gui->setParent(ui->toolGUIArea);
        gui->move(gui->geometry().topLeft());
        gui->show();

        QLayout *layout = ui->toolGUIArea->layout();
        if (!layout)
        {
          layout = new QVBoxLayout(ui->toolGUIArea);
        }
        if (layout)
        {
          layout->addWidget(gui);
          layout->activate();
        }
      }
    }
    else
    {
        this->RemoveExistingToolGui();
    }
 
  }

}


void MitkDrawingTool::RemoveExistingToolGui()
{
  if (m_LastToolGUI)
  {
    ui->toolGUIArea->layout()->removeWidget(m_LastToolGUI);
    m_LastToolGUI->setParent(nullptr);
    delete m_LastToolGUI; // will hopefully notify parent and layouts
    m_LastToolGUI = nullptr;

    QLayout *layout = ui->toolGUIArea->layout();
    if (layout)
    {
      layout->activate();
    }
  }
}

void MitkDrawingTool::CreateEmptyMask(long referenceIid)
{
  mitk::Image::Pointer referenceImage = mitk::IOUtil::Load<mitk::Image>(
      this->GetDataManager()->GetDataPath(referenceIid).toStdString()
  );

  mitk::LabelSetImage::Pointer maskImage = mitk::LabelSetImage::New();
  try
  {
      maskImage->Initialize(referenceImage);
  }
  catch (mitk::Exception& e)
  {
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      QMessageBox::information(this, "New Segmentation Session", "Could not create a new segmentation session.\n");
      return;
  }

  long uid = this->GetDataManager()->GetSubjectIdFromDataId(referenceIid);

  // Create the directory to save if it doesn't exist
  QString directoryName = this->GetDataManager()->GetSubjectPath(uid)
      + QString("/") + m_AppNameShort + QString("/")
      + m_AppNameShort + "_" + "Mask";

  if (!QDir(directoryName).exists())
  {
      QDir().mkpath(directoryName);
  }

  QString nifti = directoryName + QString("/mask.nii.gz");
  QString nrrd  = directoryName + QString("/mask.nrrd");

  // Remove previous masks
  auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(uid);

  for (const long& tIid : iids)
  {
      QString tPath = this->GetDataManager()->GetDataPath(tIid);

      if (tPath == nifti || tPath == nrrd)
      {
          this->GetDataManager()->RemoveData(tIid);
      }
  }

  // Save
  mitk::IOUtil::Save(
      maskImage,
      nifti.toStdString()
  );

  mitk::IOUtil::Save(
      maskImage,
      nrrd.toStdString()
  );

  // Update DataManager
  this->GetDataManager()->AddDataToSubject(
      uid, nifti, "Mask"
  );

  this->GetDataManager()->AddDataToSubject(
      uid, nrrd, "Mask"
  );
}

void MitkDrawingTool::OnCreateEmptyMaskBackgroundFinished()
{
  ui->newLabelPushBtn->show();
  m_ProgressDialog->cancel();
  delete m_ProgressDialog;
}