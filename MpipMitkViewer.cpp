#include "MpipMitkViewer.h"

#include <QDebug>
#include <QMouseEvent>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

MpipMitkViewer::MpipMitkViewer()
{
	this->InitializeWidget();
	m_DataStorage = m_DataStorage = mitk::StandaloneDataStorage::New();
	this->SetDataStorage(m_DataStorage);
	SetupWidgets();

	this->InitPositionTracking();
	this->EnablePositionTracking();
	auto geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);

	// Initialize bottom-right view as 3D view
	this->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

	// Enable standard handler for levelwindow-slider
	this->EnableStandardLevelWindow();

	// Add the displayed views to the DataStorage to see their positions in 2D and 3D
	this->AddDisplayPlaneSubTree();
	this->AddPlanesToDataStorage();
	this->SetWidgetPlanesVisibility(true);
}

void MpipMitkViewer::ChangeOpacity(float value)
{
	if (!lastOverlayPath.isEmpty()) {
		// TODO:
		//the float value will come from QSlider
		//find node
		//change opacity
	}	
}

bool MpipMitkViewer::RemoveImageOrOverlayIfLoaded(QString path)
{
	// TODO
	return true;
}

void MpipMitkViewer::SaveOverlayToFile(QString fullPath)
{
	// TODO
}

void MpipMitkViewer::Display(QString imagePath, QString overlayPath)
{
	// Remove the previous ones
	if (!lastImagePath.isEmpty()) {
		m_DataStorage->Remove(
			m_DataStorage->GetNamedNode( lastImagePath.toStdString().c_str() )
		);
	}
	if (!lastOverlayPath.isEmpty()) {
		m_DataStorage->Remove(
			m_DataStorage->GetNamedNode(lastOverlayPath.toStdString().c_str())
		);
	}

	lastImagePath.clear();
	lastOverlayPath.clear();

	// Load datanode (eg. many image formats, surface formats, etc.)
	if (imagePath.toStdString() != "")
	{
		qDebug() << QString("MPIP: Trying to display image...");
		mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(imagePath.toStdString(), *m_DataStorage);
		
		if (dataNodes->empty()) {
			qDebug() << QString("Could not open file: ") << imagePath;
		}
		else {
			//dataNodes->Modified();
			//m_DataStorage->Modified();
			//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
			mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
		
			mitk::DataNode::Pointer newNode = mitk::DataNode::New();
			newNode->SetName(imagePath.toStdString().c_str());
			newNode->SetData(image);
			newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
			m_DataStorage->Add(newNode);
		}

		lastImagePath = imagePath;
	}
	if (overlayPath.toStdString() != "")
	{
		qDebug() << QString("MPIP: Trying to display overlay...");
		//mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(overlayPath.toStdString(), *m_DataStorage);
		mitk::LabelSetImage::Pointer labelsImage =
			mitk::IOUtil::Load<mitk::LabelSetImage >(overlayPath.toStdString());
		// TODO: Look into: 
		// void mitk::LabelSetImage::InitializeByLabeledImage (mitk::Image::Pointer image)

		mitk::DataNode::Pointer newNode = mitk::DataNode::New();
		newNode->SetName(overlayPath.toStdString().c_str());
		newNode->SetData(labelsImage);
		newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
		m_DataStorage->Add(newNode);

		lastOverlayPath = overlayPath;
	}

	
	//this->SetDataStorage(m_DataStorage);
	//this->UpdateAllWidgets();
	this->ResetCrosshair();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	//this->RequestUpdate();
	this->ForceImmediateUpdate();
	//this->RequestUpdate();
}