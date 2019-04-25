#include "MitkImageViewer.h"

#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDir>

#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkConvert2Dto3DImageFilter.h>

#include <iostream>

MitkImageViewer::MitkImageViewer(QWidget *parent) : ImageViewerBase(parent),
	m_DataStorage(CustomMitkDataStorage::GetInstance())
{
	// Create an instance of CustomQmitkStdMultiWidget and show it
	m_MitkWidget = new CustomQmitkStdMultiWidget(this);
	m_MitkWidget->AddSlidersToViews();
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(m_MitkWidget, 0, 0);
	this->setLayout(layout);

	m_MitkWidget->InitializeWidget();
	m_MitkWidget->SetDataStorage(m_DataStorage);
	m_MitkWidget->InitPositionTracking();
	m_MitkWidget->EnablePositionTracking();
	auto geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);
	mitk::Color color;
	color = 0.50f, 0.67f, 0.8f;
	m_MitkWidget->SetDecorationProperties("Axial", color, 0);
	m_MitkWidget->SetDecorationProperties("Sagittal", color, 1);
	m_MitkWidget->SetDecorationProperties("Coronal", color, 2);
	m_MitkWidget->SetDecorationProperties("3D", color, 3);

	// Initialize bottom-right view as 3D view
	m_MitkWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

	// Enable standard handler for levelwindow-slider
	m_MitkWidget->EnableStandardLevelWindow();

	// Add the displayed views to the DataStorage to see their positions in 2D and 3D
	m_MitkWidget->AddDisplayPlaneSubTree();
	m_MitkWidget->AddPlanesToDataStorage();
	m_MitkWidget->SetWidgetPlanesVisibility(true);
}

void MitkImageViewer::OpacitySliderHandler(int value) 
{
	long iid = this->GetDataView()->GetCurrentDataID();

	if (iid == -1 || !m_DataView->IsDataChecked(iid)) { return; }

	qDebug() << "Changing opacity of " << iid <<" to value " << value;

	mitk::DataNode* node = m_DataStorage->GetNamedNode(
		std::to_string(iid)
	);

	if (node) {
		//node->SetProperty("layer", mitk::IntProperty::New(1));
		node->SetProperty("opacity", mitk::FloatProperty::New(value/100.0));
		//node->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

MitkImageViewer::~MitkImageViewer()
{
	qDebug() << "MitkImageViewer::~MitkImageViewer()";
}

void MitkImageViewer::SelectedSubjectChangedHandler(long uid) 
{
	m_MitkWidget->ResetCrosshair();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	m_FirstTimeForThisSubject = true;
}

void MitkImageViewer::DataRemovedFromSelectedSubjectHandler(long iid)
{
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	m_MitkWidget->RequestUpdate();
}

void MitkImageViewer::SelectedDataChangedHandler(long iid)
{

}

void MitkImageViewer::DataCheckedStateChangedHandler(long iid, bool checkState) 
{
	// An image got checked/unchecked in the viewer
	// (All the images of a new selected subject start unchecked)
	qDebug() << QString("MitkImageViewer::DataCheckedStateChangedHandler()") 
		<< iid << "( Special Role:" << this->GetDataManager()->GetDataSpecialRole(iid) << ")";

	mitk::DataNode::Pointer dataNode = m_DataStorage->GetNamedNode(
		std::to_string(iid)
	);

	if (!dataNode)
	{
		qDebug() << "MitkImageViewer::DataCheckedStateChangedHandler() " << "dataNode not found";
		return;
	}

	if (checkState)
	{
		QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
		if (specialRole == "Segmentation")
		{
			dataNode->SetProperty("layer", mitk::IntProperty::New(3));
		}
		else if (specialRole == "Seeds")
		{
			dataNode->SetProperty("layer", mitk::IntProperty::New(4));
		}
		else {
			dataNode->SetProperty("layer", mitk::IntProperty::New(2));
		}

		dataNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
	}
	else {
		dataNode->SetProperty("layer", mitk::IntProperty::New(1));
		dataNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));
	}

	dataNode->SetVisibility(checkState);
	if (m_FirstTimeForThisSubject)
	{
		m_MitkWidget->ResetCrosshair();
		m_FirstTimeForThisSubject = false;
	}

	dataNode->GetData()->Modified();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	m_MitkWidget->RequestUpdate();
}