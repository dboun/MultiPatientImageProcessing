#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QString>
#include <QFileInfo>
#include <QRegExp>
#include <QDebug>
#include <QtConcurrent>

#include "MitkSegmentationTool.h"
#include "ui_MitkSegmentationTool.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"
#include "QmitkNewSegmentationDialog.h"
#include <mitkIOUtil.h>
#include "QmitkToolGUI.h"

#include "DataManager.h"

MitkSegmentationTool::MitkSegmentationTool(QWidget *parent) :
    GuiModuleBase(parent),
    ui(new Ui::MitkSegmentationTool),
    m_DataStorage(&CustomMitkDataStorage::GetInstance()),
    m_LastToolGUI(nullptr)
{
    ui->setupUi(this);

    // ui->infoLabel->setText(
    //   QString("<b>1.</b> Create Mask.<br>") +
    //   QString("<b>2.</b> Draw with at least two colors.<br>") +
    //   QString("<b>3.</b> Click run and wait.<br>") +
    //   QString("<b>4.</b> If the output segmentation") +
    //   QString(" contains mistakes,") +
    //   QString(" draw over them on the mask") +
    //   QString(" with the correct color") +
    //   QString(" and run again.")
    // );

    ui->infoLabel->setText(
      QString("<b>-</b> Create new for manual segmentation.<br>")
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

    connect(ui->newLabelPushBtn,   SIGNAL(clicked()),         this, SLOT(OnAddNewLabelClicked()));
    connect(ui->createMaskPushBtn, SIGNAL(clicked()),         this, SLOT(OnCreateNewLabelSetImageClicked()));
    connect(ui->toolSelectionBox,  SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)));
    
    ui->toolGUIArea->setVisible(false);
    ui->toolSelectionBox->setVisible(false);

    m_ProgressDialogWatcher = new QFutureWatcher<void>(this);
    connect(m_ProgressDialogWatcher, SIGNAL(finished()),
      this, SLOT(OnCreateEmptyMaskBackgroundFinished())
    );

  // Create mitk node with an (all zero) 3D image for resetting
  using ImageType = itk::Image< unsigned char, 3 >;
  ImageType::Pointer image = ImageType::New();

  ImageType::IndexType start;
  start[0] = 0;  // first index on X
  start[1] = 0;  // first index on Y
  start[2] = 0;  // first index on Z

  ImageType::SizeType  size;
  size[0] = 1;  // size along X
  size[1] = 1;  // size along Y
  size[2] = 1;  // size along Z

  ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );

  image->SetRegions( region );
  image->Allocate();

  mitk::Image::Pointer emptyNormalImage = mitk::Image::New();
  emptyNormalImage->InitializeByItk<ImageType>(image);
  mitk::LabelSetImage::Pointer emptyLabelSetImage = mitk::LabelSetImage::New();
  //emptyLabelSetImage->Initialize(dynamic_cast<mitk::Image*>(m_LoadedMaskNode->GetData()));
  emptyLabelSetImage->Initialize(emptyNormalImage);
  m_EmptyImageNode = mitk::DataNode::New();
  m_EmptyImageNode->SetData(emptyLabelSetImage);
}

MitkSegmentationTool::~MitkSegmentationTool()
{
  if (m_LoadedMaskNode) { this->ChangeFocusImage(-1); }
  delete ui;
}

void MitkSegmentationTool::ChangeFocusImage(long iid)
{
  qDebug() << "MitkSegmentationTool::ChangeFocusImage";
  
  if (m_CurrentFocusImageID == iid) { return; }
  else { RevertToNullState(); }

  if (iid == -1) { return; }

  if(this->GetDataManager()->GetDataType(iid) != "LabelSetImage") { return; }
  QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
  if (specialRole != m_SpecialRoleOfInterest) { return; }

  ui->newLabelPushBtn->show();
  ui->labelSetWidget->show();

  if (!m_AllowMultiple)
  {
    ui->createMaskPushBtn->hide();
  }

  mitk::DataNode::Pointer dataNode = m_DataStorage->GetNamedNode(std::to_string(iid));
  if (!dataNode) 
  { 
    RevertToNullState(); 
    return; 
  }

  m_CurrentFocusImageID = iid;

  qDebug() << "MitkSegmentationTool::ChangeFocusImage: Changing to" << iid;
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
    this->OnAddNewLabelClicked();
  }
}

// void MitkSegmentationTool::OnDataAboutToGetRemoved(long iid)
// {
//   qDebug() << "MitkSegmentationTool::OnDataAboutToGetRemoved" << iid;

//   if (!m_MaskLoadedForThisSubject || !m_LoadedMaskNode || iid == -1)
//   {
//     return;
//   }

//   long loadedMaskIid = QString(m_LoadedMaskNode->GetName().c_str()).toLong();

//   if (loadedMaskIid == iid)
//   {
//     ui->newLabelPushBtn->hide();
//     ui->labelSetWidget->hide();
//     ui->createMaskPushBtn->show();

//     emit MitkSegmentationToolSaveImageToFile(loadedMaskIid, false);

//     m_MaskLoadedForThisSubject = false;
    
//     ui->toolGUIArea->setVisible(false);
//     ui->toolSelectionBox->setVisible(false);

//     mitk::LabelSetImage::Pointer emptyImage = mitk::LabelSetImage::New();
//     emptyImage->Initialize(dynamic_cast<mitk::Image*>(m_LoadedMaskNode->GetData()));
//     mitk::DataNode::Pointer emptyNode = mitk::DataNode::New();
//     emptyNode->SetData(emptyImage);
//     //m_DataStorage->Add(emptyNode);

//     m_ToolManager->SetWorkingData(emptyNode);
//     m_ToolManager->SetReferenceData(emptyNode); 

//     //ui->labelSetWidget->UpdateAllTableWidgetItems();
//     ui->labelSetWidget->ResetAllTableWidgetItems();
    
//     m_LoadedMaskNode = nullptr;
//   }

//   qDebug() << "MitkSegmentationTool::OnDataAboutToGetRemovedFinished";
// }

void MitkSegmentationTool::SetAllowMultiple(bool allowMultiple)
{
  m_AllowMultiple = allowMultiple;
}

void MitkSegmentationTool::RevertToNullState()
{
  m_CurrentFocusImageID = -1;
  
  ui->newLabelPushBtn->hide();
  ui->labelSetWidget->hide();
  ui->createMaskPushBtn->show();

  m_MaskLoadedForThisSubject = false;
  
  ui->toolGUIArea->setVisible(false);
  ui->toolSelectionBox->setVisible(false);
  
  m_ToolManager->SetWorkingData(m_EmptyImageNode);
  m_ToolManager->SetReferenceData(m_EmptyImageNode); 

  //ui->labelSetWidget->UpdateAllTableWidgetItems();
  ui->labelSetWidget->ResetAllTableWidgetItems();
  
  m_LoadedMaskNode = nullptr;
}

void MitkSegmentationTool::SetSpecialRoleOfInterest(QString specialRoleOfInterest)
{
  m_SpecialRoleOfInterest = specialRoleOfInterest;
}

void MitkSegmentationTool::OnAddNewLabelClicked()
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

  if (ui->labelSetWidget->isHidden()) { ui->labelSetWidget->show(); }
  ui->labelSetWidget->ResetAllTableWidgetItems();

  mitk::RenderingManager::GetInstance()->InitializeViews(workingNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
}

void MitkSegmentationTool::OnCreateNewLabelSetImageClicked()
{   
    // -1 means to the current subject
    long iid = m_DataStorage->AddEmptyMitkLabelSetImageToSubject(
      -1, m_SpecialRoleOfInterest
    );
    
    if (iid == -1)
    {
      QMessageBox::information(this, 
        "Drawing Tool", 
        "Please load a subject before starting some action."
      );
      return;
    }

    if (!m_AllowMultiple)
    {
      ui->createMaskPushBtn->hide();
    }
    
    // m_ProgressDialog = new QProgressDialog(this);
    // m_ProgressDialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | 
    //   Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint
    // );
    // m_ProgressDialog->setWindowTitle("Please wait");
    // m_ProgressDialog->setCancelButton(nullptr);
    // //Set progress dialog in undeterminate state: 
    // m_ProgressDialog->setMaximum( 0 );
    // m_ProgressDialog->setMinimum( 0 );
    // m_ProgressDialog->show();

    /*QFuture<void> future = QtConcurrent::run(
      this, &MitkSegmentationTool::CreateEmptyMask, 
      referenceIid
    );
    m_ProgressDialogWatcher->setFuture(future);*/
}

// void MitkSegmentationTool::SetMaskFromNiftiData(long iid)
// {
//   mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>(
//       this->GetDataManager()->GetDataPath(iid).toStdString()
//   );
//   mitk::LabelSetImage::Pointer maskImage = mitk::LabelSetImage::New();
	
// 	// Copy the data from input image
// 	maskImage->InitializeByLabeledImage(inputImage);

//   long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);

//   // Create the directory to save if it doesn't exist
//   QString directoryName = this->GetDataManager()->GetSubjectPath(uid)
//       + QString("/") + m_AppNameShort + QString("/")
//       + m_AppNameShort + "_" + "Mask";

//   if (!QDir(directoryName).exists())
//   {
//       QDir().mkpath(directoryName);
//   }

//   //QString nifti = directoryName + QString("/mask.nii.gz");
//   QString nrrd  = directoryName + QString("/mask.nrrd");
  
//   // Remove previous masks
//   auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(uid);

//   // Save
//   // mitk::IOUtil::Save(
//   //     maskImage,
//   //     nifti.toStdString()
//   // );

//   mitk::IOUtil::Save(
//       maskImage,
//       nrrd.toStdString()
//   );

//   // // Update DataManager
//   // this->GetDataManager()->AddDataToSubject(
//   //     uid, nifti, "Mask"
//   // );

//   this->GetDataManager()->AddDataToSubject(
//       uid, nrrd, "Mask", "Image", "<Mask>"
//   );

//   this->GetDataManager()->RemoveData(iid);

//   for (const long& tIid : iids)
//   {
//     if (tIid == iid) { continue; }

//     QString tPath = this->GetDataManager()->GetDataPath(tIid);

//     if (/*tPath == nifti || */tPath == nrrd)
//     {
//       this->GetDataManager()->RemoveData(tIid);
//     }
//   }
// }

// void MitkSegmentationTool::SetSegmentationFromNiftiData(long iid)
// {
//   mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>(
//       this->GetDataManager()->GetDataPath(iid).toStdString()
//   );
//   mitk::LabelSetImage::Pointer segImage = mitk::LabelSetImage::New();
	
// 	// Copy the data from input image
// 	segImage->InitializeByLabeledImage(inputImage);

//   long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);

//   // Create the directory to save if it doesn't exist
//   QString directoryName = this->GetDataManager()->GetSubjectPath(uid)
//       + QString("/") + m_AppNameShort + QString("/")
//       + m_AppNameShort + "_" + "Segmentation";

//   if (!QDir(directoryName).exists())
//   {
//       QDir().mkpath(directoryName);
//   }

//   QString nrrd, name;
//   QFileInfo f(this->GetDataManager()->GetDataPath(iid));

//   if (this->GetDataManager()->GetDataSpecialRole(iid) != "Segmentation")
//   {
//     name = "<Segmentation> (" + f.baseName() + ")";
//     nrrd = directoryName + QString("/") + f.baseName() + QString("_segmentation.nrrd");
//   }
//   else {
//     name = this->GetDataManager()->GetDataName(iid);
//     nrrd = directoryName + QString("/") + f.baseName() + ".nrrd";
//   }

//   // Copy the labels from reference image
//   if (m_LoadedMaskNode)
// 	{
//     mitk::LabelSet::Pointer referenceLabelSet =	dynamic_cast<mitk::LabelSetImage*>(
//       m_LoadedMaskNode->GetData()
//     )->GetActiveLabelSet();
//     mitk::LabelSet::Pointer outputLabelSet    =	segImage->GetActiveLabelSet();

//     mitk::LabelSet::LabelContainerConstIteratorType itR;
//     mitk::LabelSet::LabelContainerConstIteratorType it;
    
//     for (itR =  referenceLabelSet->IteratorConstBegin();
//         itR != referenceLabelSet->IteratorConstEnd(); 
//         ++itR) 
//     {
//       for (it = outputLabelSet->IteratorConstBegin(); 
//           it != outputLabelSet->IteratorConstEnd();
//           ++it)
//       {
//         if (itR->second->GetValue() == it->second->GetValue())
//         {
//           it->second->SetColor(itR->second->GetColor());
//           it->second->SetName(itR->second->GetName());
//         }
//       }
//     }
//   }
  
//   mitk::IOUtil::Save(
//       segImage,
//       nrrd.toStdString()
//   );

//   this->GetDataManager()->AddDataToSubject(
//       uid, nrrd, "Segmentation", "Image", name
//   );

//   this->GetDataManager()->RemoveData(iid);
// }

void MitkSegmentationTool::OnManualTool2DSelected(int id)
{
  qDebug() << "MitkSegmentationTool::OnManualTool2DSelected";

  if (!m_MaskLoadedForThisSubject)
  {
    return;
  }

  qDebug() << "MitkSegmentationTool::OnManualTool2DSelected (mask is loaded)";

  if (id >= 0)
  {
    std::string text = m_ToolManager->GetToolById(id)->GetName();
    
    if (text == "Paint")
    {
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


void MitkSegmentationTool::RemoveExistingToolGui()
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

// void MitkSegmentationTool::CreateEmptyMask(long referenceIid)
// {
//   mitk::Image::Pointer referenceImage = mitk::IOUtil::Load<mitk::Image>(
//     this->GetDataManager()->GetDataPath(referenceIid).toStdString()
//   );

//   mitk::LabelSetImage::Pointer maskImage = mitk::LabelSetImage::New();
//   try
//   {
//     maskImage->Initialize(referenceImage);
//   }
//   catch (mitk::Exception& e)
//   {
//     MITK_ERROR << "Exception caught: " << e.GetDescription();
//     QMessageBox::information(this, "New Segmentation Session", "Could not create a new segmentation session.\n");
//     return;
//   }

//   long uid = this->GetDataManager()->GetSubjectIdFromDataId(referenceIid);

//   // Create the directory to save if it doesn't exist
//   QString directoryName = this->GetDataManager()->GetSubjectPath(uid)
//       + QString("/") + m_AppNameShort + QString("/")
//       + m_AppNameShort + "_" + m_SpecialRoleOfInterest;

//   if (!QDir(directoryName).exists())
//   {
//       QDir().mkpath(directoryName);
//   }

//   //QString nifti = directoryName + QString("/mask.nii.gz");
//   QString nrrd  = directoryName + QString("/" + m_SpecialRoleOfInterest.toLower() + "nrrd");
//   // TODO: Not overwrite previous segmentations

//   // Remove previous masks
//   auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(uid);

//   for (const long& tIid : iids)
//   {
//       QString tPath = this->GetDataManager()->GetDataPath(tIid);

//       if (/*tPath == nifti || */tPath == nrrd)
//       {
//           this->GetDataManager()->RemoveData(tIid);
//       }
//   }

//   // Save
//   // mitk::IOUtil::Save(
//   //     maskImage,
//   //     nifti.toStdString()
//   // );

//   mitk::IOUtil::Save(
//       maskImage,
//       nrrd.toStdString()
//   );

//   // Update DataManager
//   // this->GetDataManager()->AddDataToSubject(
//   //     uid, nifti, "Mask"
//   // );

//   this->GetDataManager()->AddDataToSubject(
//       uid, nrrd, m_SpecialRoleOfInterest, "Image", "<" + m_SpecialRoleOfInterest + ">"
//   );
// }

// void MitkSegmentationTool::OnCreateEmptyMaskBackgroundFinished()
// {
//   ui->newLabelPushBtn->show();
//   m_ProgressDialog->cancel();
//   delete m_ProgressDialog;
// }