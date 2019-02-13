#include "MpipMitkViewer.h"

MpipMitkViewer::MpipMitkViewer()
{
	this->InitializeWidget();
	m_DataStorage = m_DataStorage = mitk::StandaloneDataStorage::New();
	this->SetDataStorage(m_DataStorage);

	
}

void MpipMitkViewer::SetupWidgets()
{
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

void MpipMitkViewer::Display(QString imagePath)
{
	// Load datanode (eg. many image formats, surface formats, etc.)
	if (imagePath.toStdString() != "")
	{
		qDebug() << QString("MPIP: Trying to display...");
		//mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(imagePath.toStdString()/*, *m_DataStorage*/);
		mitk::IOUtil::Load(imagePath.toStdString(), *m_DataStorage);		
		/*dataNodes->at(0)->SetProperty("binary", mitk::BoolProperty::New(true));
		dataNodes->at(0)->SetProperty("name", mitk::StringProperty::New("test segmentation"));
		dataNodes->at(0)->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
		dataNodes->at(0)->SetProperty("volumerendering", mitk::BoolProperty::New(true));
		dataNodes->at(0)->SetProperty("layer", mitk::IntProperty::New(1));
		dataNodes->at(0)->SetProperty("opacity", mitk::FloatProperty::New(0.5));*/


		/*if (dataNodes->empty()) {
			qDebug() << QString("Could not open file: ") << imagePath;
		}
		else {
			dataNodes->Modified();
			m_DataStorage->Modified();
			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
			mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
		
			mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
			pointSetNode->SetData(image);
			pointSetNode->SetProperty("layer", mitk::IntProperty::New(2));
			m_DataStorage->Add(pointSetNode);
		}*/
	}

	
	//this->SetDataStorage(m_DataStorage);
	//this->UpdateAllWidgets();
	SetupWidgets();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	this->RequestUpdate();
}