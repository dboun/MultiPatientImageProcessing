#define SMW_INFO MITK_INFO("widget.stdmulti")

#include "CustomQmitkStdMultiWidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QMouseEvent>
#include <QTimer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <qsplitter.h>

#include "mitkImagePixelReadAccessor.h"
#include "mitkPixelTypeMultiplex.h"
#include <mitkManualPlacementAnnotationRenderer.h>
#include <mitkCameraController.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>
#include <mitkNodePredicateBase.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkStatusBar.h>
#include <mitkVtkLayerController.h>
#include <vtkSmartPointer.h>
#include <vtkQImageToImageSource.h>
#include <vtkCornerAnnotation.h>
#include <vtkMitkRectangleProp.h>
#include <vtkTextProperty.h>

#include <iomanip>

#include "QmitkSliceWidget.h"
#include <QDebug>

CustomQmitkStdMultiWidget::CustomQmitkStdMultiWidget(QWidget *parent,
                                         Qt::WindowFlags f,
                                         mitk::RenderingManager *renderingManager,
                                         mitk::BaseRenderer::RenderingMode::Type renderingMode,
                                         const QString &name)
  : QmitkStdMultiWidget(parent, f, renderingManager, renderingMode, name)
{

}

CustomQmitkStdMultiWidget::~CustomQmitkStdMultiWidget()
{
  DisablePositionTracking();
  // DisableNavigationControllerEventListening();

  m_TimeNavigationController->Disconnect(mitkWidget1->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget2->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget3->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget4->GetSliceNavigationController());
}

void CustomQmitkStdMultiWidget::AddSlidersToViews(
  mitk::RenderingManager *renderingManager,
  mitk::BaseRenderer::RenderingMode::Type renderingMode,
  const QString &name)
{
  // ------ Remove old things ------
  qDebug() << "CustomQmitkStdMultiwidget: Removing all things...";
  m_TimeNavigationController->Disconnect(mitkWidget1->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget2->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget3->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget4->GetSliceNavigationController());

  m_SubSplit1->widget(m_SubSplit1->count()-1)->deleteLater();
  m_SubSplit1->widget(m_SubSplit1->count()-1)->deleteLater();
  m_SubSplit2->widget(m_SubSplit2->count()-1)->deleteLater();
  m_SubSplit2->widget(m_SubSplit2->count()-1)->deleteLater();

  delete mitkWidget1;
  delete mitkWidget2;
  delete mitkWidget3;
  delete mitkWidget4;

  // ------ Recreate Widget Layouts ------
  qDebug() << "CustomQmitkStdMultiwidget: Recreating layouts...";
  mitkWidget1Container = new QWidget(m_SubSplit1);
  mitkWidget2Container = new QWidget(m_SubSplit1);
  mitkWidget3Container = new QWidget(m_SubSplit2);
  mitkWidget4Container = new QWidget(m_SubSplit2);
  QHBoxLayout *mitkWidgetLayout1 = new QHBoxLayout(mitkWidget1Container);
  QHBoxLayout *mitkWidgetLayout2 = new QHBoxLayout(mitkWidget2Container);
  QHBoxLayout *mitkWidgetLayout3 = new QHBoxLayout(mitkWidget3Container);
  QVBoxLayout *mitkWidgetLayout4 = new QVBoxLayout(mitkWidget4Container);
  mitkWidget1Container->setLayout(mitkWidgetLayout1);
  mitkWidget1Container->setLayout(mitkWidgetLayout2);
  mitkWidget1Container->setLayout(mitkWidgetLayout3);
  mitkWidget1Container->setLayout(mitkWidgetLayout4);

  // ------ Recreate the important things ------
  qDebug() << "CustomQmitkStdMultiwidget: Recreating the important things...";

  // Recreate Window 1
  QmitkSliceWidget* mitkWidgetWithSliceNav1 = new QmitkSliceWidget(mitkWidget1Container, ".widget1");
  mitkWidgetWithSliceNav1->findChild<QmitkLevelWindowWidget*>("levelWindow")->hide();
  mitkWidget1 = mitkWidgetWithSliceNav1->GetRenderWindow();
  //mitkWidget1 = new QmitkRenderWindow(mitkWidget1Container, name + ".widget1", nullptr, m_RenderingManager, renderingMode);
  mitkWidget1->SetLayoutIndex(AXIAL);
  mitkWidgetLayout1->addWidget(mitkWidgetWithSliceNav1);
  mitkWidgetLayout1->setContentsMargins(QMargins(0,0,0,0));

  // Recreate Window 2
  QmitkSliceWidget* mitkWidgetWithSliceNav2 = new QmitkSliceWidget(mitkWidget2Container, ".widget2");
  mitkWidgetWithSliceNav2->findChild<QmitkLevelWindowWidget*>("levelWindow")->hide();
  mitkWidget2 = mitkWidgetWithSliceNav2->GetRenderWindow();
  //mitkWidget2 = new QmitkRenderWindow(mitkWidget2Container, name + ".widget2", nullptr, m_RenderingManager, renderingMode);
  mitkWidget2->setEnabled(true);
  mitkWidget2->SetLayoutIndex(SAGITTAL);
  mitkWidgetLayout2->addWidget(mitkWidgetWithSliceNav2);
  mitkWidgetLayout2->setContentsMargins(QMargins(0,0,0,0));

  // Recreate Window 3
  QmitkSliceWidget* mitkWidgetWithSliceNav3 = new QmitkSliceWidget(mitkWidget3Container, ".widget3");
  mitkWidgetWithSliceNav3->findChild<QmitkLevelWindowWidget*>("levelWindow")->hide();
  mitkWidget3 = mitkWidgetWithSliceNav3->GetRenderWindow();
  //mitkWidget3 = new QmitkRenderWindow(mitkWidget3Container, name + ".widget3", nullptr, m_RenderingManager, renderingMode);
  mitkWidget3->SetLayoutIndex(CORONAL);
  mitkWidgetLayout3->addWidget(mitkWidgetWithSliceNav3);
  mitkWidgetLayout3->setContentsMargins(QMargins(0,0,0,0));

  // Recreate RenderWindow 4
  mitkWidget4 = new QmitkRenderWindow(mitkWidget4Container, name + ".widget4", nullptr, m_RenderingManager, renderingMode);
  mitkWidget4->SetLayoutIndex(THREE_D);
  mitkWidgetLayout4->addWidget(mitkWidget4);
  mitkWidgetLayout4->setContentsMargins(QMargins(0,0,2,/*25*/0));
  
  // Reset button under RenderWindow 4
  QPushButton* reset = new QPushButton(this);
  reset->setText("Reset level/window");
  QFont font = reset->font();
  font.setPointSize(7);
  reset->setFont(font);
  reset->resize(reset->width(), 20);
  connect(reset, SIGNAL(clicked()), this, SLOT(OnResetButtonPressed()));
  mitkWidgetLayout4->addWidget(reset, 0, Qt::AlignRight);
  
  // ------ Reconnect Signals and Slots ------
  connect(mitkWidget1, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(OnLayoutDesignChanged(int)));
  connect(mitkWidget1, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(CustomOnLayoutDesignChanged(int)));
  connect(mitkWidget1, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(mitkWidget1, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));
  connect(this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget1, SLOT(OnWidgetPlaneModeChanged(int)));

  connect(mitkWidget2, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(OnLayoutDesignChanged(int)));
  connect(mitkWidget2, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(CustomOnLayoutDesignChanged(int)));
  connect(mitkWidget2, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(mitkWidget2, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));
  connect(this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget2, SLOT(OnWidgetPlaneModeChanged(int)));

  connect(mitkWidget3, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(OnLayoutDesignChanged(int)));
  connect(mitkWidget3, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(CustomOnLayoutDesignChanged(int)));
  connect(mitkWidget3, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(mitkWidget3, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));
  connect(this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget3, SLOT(OnWidgetPlaneModeChanged(int)));

  connect(mitkWidget4, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(OnLayoutDesignChanged(int)));
  connect(mitkWidget4, SIGNAL(SignalLayoutDesignChanged(int)), this, SLOT(CustomOnLayoutDesignChanged(int)));
  connect(mitkWidget4, SIGNAL(ResetView()), this, SLOT(ResetCrosshair()));
  connect(mitkWidget4, SIGNAL(ChangeCrosshairRotationMode(int)), this, SLOT(SetWidgetPlaneMode(int)));
  connect(this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget4, SLOT(OnWidgetPlaneModeChanged(int)));

  // ------ Finish touches ------

  // Resize
  this->resize(QSize(364, 477).expandedTo(minimumSizeHint()));

  // Reinitialize the widgets
  //this->InitializeWidget();

  // Activate Widget Menu
  this->ActivateMenuWidget(true);

  // Fixes 3D view being small the first time
  this->changeLayoutToDefault();
  this->CustomOnLayoutDesignChanged(LAYOUT_DEFAULT);
}

void CustomQmitkStdMultiWidget::CustomOnLayoutDesignChanged(int layoutDesignIndex)
{
  this->resize(QSize(364, 477).expandedTo(minimumSizeHint()));
}

void CustomQmitkStdMultiWidget::OnResetButtonPressed()
{
  qDebug() << "CustomQmitkStdMultiWidget::OnResetButtonPressed";
  mitk::LevelWindow lw;
  try {
    lw = levelWindowWidget->GetManager()->GetLevelWindow();
  } catch (itk::ExceptionObject e) {
    return; // There is no level window
  }
  
  bool isFixed = lw.IsFixed();
  lw.SetFixed(false);
  lw.ResetDefaultRangeMinMax();
  lw.ResetDefaultLevelWindow();
  //lw.SetWindowBounds(lw.GetDefaultLowerBound(), lw.GetDefaultUpperBound());
  //lw.SetLevelWindow(lw.GetDefaultLevel(), lw.GetDefaultWindow());
  //lw.SetAuto();
  lw.SetFixed(isFixed);
  levelWindowWidget->GetManager()->SetLevelWindow(lw);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}