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
  m_DataStorage(CustomMitkDataStorage::GetInstance()),
  m_LastToolGUI(nullptr)
{
  ui->setupUi(this);

  ui->infoLabel->setText(
    QString(" &#8226; Create new for manual segmentation<br>") +
    QString(" &#8226; Select a segmentation in the data view to edit/view it<br>")
  );

  ui->newLabelPushBtn->hide();
  ui->labelSetWidget->hide();
  ui->createMaskPushBtn->show();

  this->m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    // mitk::ToolManager::New(nullptr);
  assert(m_ToolManager);

  m_ToolManager->SetDataStorage(*m_DataStorage);
  m_ToolManager->InitializeTools();

  ui->labelSetWidget->SetDataStorage(m_DataStorage);
  ui->labelSetWidget->SetOrganColors(mitk::OrganNamesHandling::GetDefaultOrganColorString());
  ui->labelSetWidget->findChild<ctkSearchBox*>("m_LabelSearchBox")->hide();
  connect(ui->labelSetWidget, SIGNAL(goToLabel(const mitk::Point3D&)),
    this, SLOT(OnMitkGoToLabel(const mitk::Point3D&))
  );
  // auto pbVisible = ui->labelSetWidget->findChild<QPushButton*>("pbVisible");
  // connect(pbVisible, SIGNAL(clicked()), this, SLOT(Refresh()));
  // auto pbColor = ui->labelSetWidget->findChild<QPushButton*>("pbColor");
  // connect(pbColor, SIGNAL(clicked()), this, SLOT(Refresh()));

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
  emptyLabelSetImage->Initialize(emptyNormalImage);
  m_EmptyImageNode = mitk::DataNode::New();
  m_EmptyImageNode->SetData(emptyLabelSetImage);
  m_EmptyImageNode->SetName(std::to_string(-1));
  this->m_ToolManager->SetWorkingData(m_EmptyImageNode);
  this->m_ToolManager->SetReferenceData(m_EmptyImageNode);
}

MitkSegmentationTool::~MitkSegmentationTool()
{
  qDebug() << "MitkSegmentationTool::~MitkSegmentationTool()";
  //if (m_LoadedMaskNode) { this->ChangeFocusImage(-1); }
  delete ui;
}

void MitkSegmentationTool::ChangeFocusImage(long iid)
{
  qDebug() << "MitkSegmentationTool::ChangeFocusImage" << iid;
  
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
  ui->labelSetWidget->UpdateControls();
  ui->labelSetWidget->ResetAllTableWidgetItems();
  
  m_LoadedMaskNode = nullptr;
}

void MitkSegmentationTool::SetSpecialRoleOfInterest(QString specialRoleOfInterest)
{
  m_SpecialRoleOfInterest = specialRoleOfInterest;
}

void MitkSegmentationTool::SetEnabled(bool enabled)
{
  if (enabled)
  {
    this->m_ToolManager->SetWorkingData(m_LoadedMaskNode);
    this->m_ToolManager->SetReferenceData(m_LoadedMaskNode);
  }
}

void MitkSegmentationTool::SetDataView(DataViewBase* dataView)
{
  m_DataView = dataView;
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

  // Here is good place to see the clicks on the eye etc
  //QObjectList widgetList = ui->labelSetWidget->layout()->children();
}

void MitkSegmentationTool::OnMitkGoToLabel(const mitk::Point3D &)
{
  // pretty much refreshing just to be sure
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void MitkSegmentationTool::Refresh()
{
  // To refresh when the 'eye' and other LabelSetWidget widgets are clicked
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
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
    
    // Check the image in data view
    if (m_DataView) { m_DataView->SetDataCheckedState(m_CurrentFocusImageID, true, true); }

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