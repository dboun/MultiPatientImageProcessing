#include "MPIPQmitkSegmentationPanel.h"
#include "ui_MPIPQmitkSegmentationPanel.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"
#include "QmitkNewSegmentationDialog.h"
#include <mitkIOUtil.h>
#include "QmitkToolGUI.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDebug>

#include "DataManager.h"

MPIPQmitkSegmentationPanel::MPIPQmitkSegmentationPanel(mitk::DataStorage *datastorage, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MPIPQmitkSegmentationPanel),
    m_DataStorage(datastorage),
    m_LastToolGUI(nullptr)
{
    ui->setupUi(this);

    this->toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    assert(toolManager);

    toolManager->SetDataStorage(*m_DataStorage);
    toolManager->InitializeTools();

    ui->labelSetWidget->SetDataStorage(m_DataStorage);
    ui->labelSetWidget->SetOrganColors(mitk::OrganNamesHandling::GetDefaultOrganColorString());
    ui->labelSetWidget->hide();

    ui->toolSelectionBox->SetToolManager(*toolManager);
    ui->toolSelectionBox->setEnabled(true);
    ui->toolSelectionBox->SetGenerateAccelerators(true);
    ui->toolSelectionBox->SetDisplayedToolGroups(tr("Add Subtract Correction Paint Erase").toStdString());
    ui->toolSelectionBox->SetLayoutColumns(3);
    ui->toolSelectionBox->SetEnabledMode(QmitkToolSelectionBox::AlwaysEnabled);

    connect(ui->newLabelPushBtn, SIGNAL(clicked()), this, SLOT(OnCreateNewLabel()));
    connect(ui->ConfirmSegBtn, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()));
    //connect(ui->toolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)));
    
}

MPIPQmitkSegmentationPanel::~MPIPQmitkSegmentationPanel()
{
    delete ui;
}

void MPIPQmitkSegmentationPanel::SetDataManager(DataManager * dataManager)
{
  this->m_DataManager = dataManager;
}

void MPIPQmitkSegmentationPanel::SetDataView(DataViewBase* dataView)
{
  this->m_DataView = dataView;
}

void MPIPQmitkSegmentationPanel::SetAppName(QString appName)
{
  m_AppName = appName;
}

// void MPIPQmitkSegmentationPanel::CreateNewSegmentation()
// {
//   // Create empty segmentation working image
//   mitk::DataNode::Pointer workingImageNode = mitk::DataNode::New();
//   mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingImageNode->GetData());
//   const std::string organName = "test";
//   mitk::Color color; // actually it dosn't matter which color we are using
//   color.SetRed(1);   // but CreateEmptySegmentationNode expects a color parameter
//   color.SetGreen(0);
//   color.SetBlue(0);
//   mitk::Tool* firstTool = toolManager->GetToolById(0);
//   workingImageNode = firstTool->CreateEmptySegmentationNode(workingImage, organName, color);
//   this->m_DataStorage->Add(workingImageNode);
//   if (workingImageNode.IsNotNull())
//     this->toolManager->SetWorkingData(workingImageNode);
//   mitk::DataNode::Pointer origNode = this->m_DataStorage->GetNamedNode("LoadedData");
//   if (origNode.IsNotNull())
//     this->toolManager->SetReferenceData(origNode);
//   mitk::RenderingManager::GetInstance()->InitializeViews(workingImageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
// }

void MPIPQmitkSegmentationPanel::OnNewSegmentationSession()
{
  segName.clear();

  // Find if there is a mask already
  bool foundMask = false;
  for (const auto& iid : m_DataManager->GetAllDataIdsOfSubject(m_DataView->GetCurrentSubjectID()))
  {
    if (m_DataManager->GetDataSpecialRole(iid) == "Mask")
    {
      foundMask = true;
      break;
    }
  }

  if (foundMask) { 
    this->OnResumeSegmentationSession(); 
    return; 
  }

  // Get desired filename
  segName = QString::fromStdString(m_DataManager->GetSubjectName(
    m_DataManager->GetSubjectIdFromDataId(m_CurrentData)
  ).toStdString());
  segName.append("-mask");

  bool ok = false;
  segName = QInputDialog::getText(this, "New Segmentation Session", "New name:", 
    QLineEdit::Normal, segName, &ok
  );

  if (!ok)
  {
    return;
  }

  if (segName == "")
  {
    segName = "mask";
  }

  InitializeReferenceNodeAndImageForSubject(m_DataView->GetCurrentSubjectID());

  // Create the image
  m_WorkingImage = mitk::LabelSetImage::New();
  try
  {
    m_WorkingImage->Initialize(m_ReferenceImage);
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "New Segmentation Session", 
      "Could not create a new segmentation session.\n"
    );
    return;
  }

  // Save the image to file
  QString maskFilename = m_DataManager->GetSubjectPath(m_DataView->GetCurrentSubjectID()) 
    + QString("/Segmentation/mask/")
    + segName
    + QString(".nrrd");

  mitk::IOUtil::Save(m_WorkingImage, maskFilename.toStdString());
  m_DataManager->AddDataToSubject(m_DataView->GetCurrentSubjectID(), maskFilename, "Mask");
}

void MPIPQmitkSegmentationPanel::OnResumeSegmentationSession()
{
  InitializeReferenceNodeAndImageForSubject(m_DataView->GetCurrentSubjectID());

  long iidMask = -1;
  for (const auto& iid : m_DataManager->GetAllDataIdsOfSubject(m_DataView->GetCurrentSubjectID()))
  {
    if (m_DataManager->GetDataSpecialRole(iid) == "Mask")
    {
      iidMask = iid;
      break;
    }
  }

  if (iidMask == -1) { return; }

  // Load the mask
  QString maskPath = m_DataManager->GetDataPath(iidMask);

  mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = 
    mitk::IOUtil::Load(maskPath.toStdString(), *m_DataStorage);
  if (dataNodes->empty()) {
    qDebug() << QString("Could not open file: ") << maskPath;
    delete dataNodes;
    return;
  }
  m_WorkingImage = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());

  toolManager->ActivateTool(-1);
  m_WorkingNode = mitk::DataNode::New();
  m_WorkingNode->SetData(m_WorkingImage);
  QFileInfo f(maskPath);
  QString maskName = f.baseName();
  m_WorkingNode->SetName(segName.toStdString());

  if (!this->m_DataStorage->Exists(m_WorkingNode))
  {
    this->m_DataStorage->Add(m_WorkingNode, m_ReferenceNode);
  }

  this->toolManager->SetWorkingData(m_WorkingNode);
  this->toolManager->SetReferenceData(m_ReferenceNode);
}

void MPIPQmitkSegmentationPanel::OnConfirmSegmentation()
{
  QString pathToSave = QDir::currentPath();
  if (m_DataManager->GetSubjectIdFromDataId(m_CurrentData) != -1)
  {
    pathToSave = m_DataManager->GetSubjectPath(m_DataManager->GetSubjectIdFromDataId(m_CurrentData));
  }

  QString filename = QFileDialog::getSaveFileName(this, tr("Save Segmentation Mask"),
    pathToSave, tr("Images (*.nrrd)"));
  
  // if (!filename.endsWith(".nii.gz"))
  // {
	 //  filename = filename + ".nii.gz";
  // }

  if (!filename.isEmpty())
  {
    mitk::DataNode::Pointer segData = this->m_DataStorage->GetNamedNode(segName.toStdString());
    mitk::IOUtil::Save(segData->GetData(), filename.toStdString());

    //remove after saving
    m_DataStorage->Remove(m_DataStorage->GetNamedNode(segName.toStdString()));
    
    if (m_DataManager->GetSubjectIdFromDataId(m_CurrentData) != -1)
    {
		m_DataManager->AddDataToSubject(
			m_DataManager->GetSubjectIdFromDataId(m_CurrentData),
			filename,
			"Mask"
		);
    }
  }

  //re-render here
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //hide ourself
  this->hide();
}

void MPIPQmitkSegmentationPanel::OnCreateNewLabel()
{
  toolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = toolManager->GetWorkingData(0);
  if (!workingNode)
  {
    QMessageBox::information(
      this, "New Segmentation Session", "Please load and select a patient image before starting some action.");
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

  int dialogReturnValue = dialog->exec();
  if (dialogReturnValue == QDialog::Rejected)
  {
    return;
  }

  QString segName = dialog->GetSegmentationName();
  if (segName.isEmpty())
  {
    segName = "Unnamed";
  }
  workingImage->GetActiveLabelSet()->AddLabel(segName.toStdString(), dialog->GetColor());

  if (ui->labelSetWidget->isHidden())
    ui->labelSetWidget->show();
  ui->labelSetWidget->ResetAllTableWidgetItems();

  mitk::RenderingManager::GetInstance()->InitializeViews(workingNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
}

void MPIPQmitkSegmentationPanel::SetDisplayDataName(long iid)
{
  this->m_CurrentData = iid;
}

// void MPIPQmitkSegmentationPanel::OnManualTool2DSelected(int id)
// {
//   if (id >= 0)
//   {
//     std::string text = toolManager->GetToolById(id)->GetName();
    
//     if (text == "Paint")
//     {
//       this->OnDisableSegmentation();

//       mitk::Tool *tool = toolManager->GetActiveTool();

//       itk::Object::Pointer possibleGUI = tool->GetGUI("Qmitk", "GUI").GetPointer(); 
//       QmitkToolGUI *gui = dynamic_cast<QmitkToolGUI *>(possibleGUI.GetPointer());
//       m_LastToolGUI = gui;
//       if (gui)
//       {
//         gui->SetTool(tool);

//         gui->setParent(ui->toolGUIArea);
//         gui->move(gui->geometry().topLeft());
//         gui->show();

//         QLayout *layout = ui->toolGUIArea->layout();
//         if (!layout)
//         {
//           layout = new QVBoxLayout(ui->toolGUIArea);
//         }
//         if (layout)
//         {
//           layout->addWidget(gui);
//           layout->activate();
//         }
//       }
//     }
//     else
//     {
//       this->OnDisableSegmentation();
//     }
 
//   }

// }

void MPIPQmitkSegmentationPanel::InitializeReferenceNodeAndImageForSubject(long uid)
{
  auto randomIid = m_DataManager->GetAllDataIdsOfSubject(uid)[0];
  QString path = m_DataManager->GetDataPath(randomIid);

  mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = 
    mitk::IOUtil::Load(path.toStdString(), *m_DataStorage);

  if (dataNodes->empty()) {
    qDebug() << QString("Could not open file: ") << path;
    delete dataNodes;
    return;
  }

  m_ReferenceImage = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
  m_ReferenceNode = mitk::DataNode::New();
  m_ReferenceNode->SetData(m_ReferenceImage);
  m_ReferenceNode->SetName(QString("reference").toStdString().c_str());
  m_ReferenceNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
  m_DataStorage->Add(m_ReferenceNode);
}