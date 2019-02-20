#include "MitkViewer.h"

#include <QDebug>
#include <QMouseEvent>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

void MitkViewer::SliderHandler(long sliderNumber, int value) 
{
	// The opacity slider will have sliderNumber == 0
	// and concerns the most focused image I guess.
	// Other sliderNumbers can be used for other
	// slider, like for changing slices 

	// We should probably leave this for last
}

void MitkViewer::SelectedSubjectChangedHandler(long uid) 
{
	// This means that a *new* subject is selected

	// Get all the data ids of this subject
	// for now assume that all the data are images
	std::vector<long> allDataOfThisSubject = m_DataManager->GetAllDataIdsOfSubject(uid);

	std::vector<QString> allImagesPaths;

	for(long& iid : allDataOfThisSubject)
	{
		allImagesPaths.push_back(
			m_DataManager->GetDataPath(iid);
		);
	}

	// allImagesPaths now contain all the paths of 
	// all the images of *this* subject

	// TODO: Stuff :P
}

void DataAddedForSelectedSubjectHandler(long iid)
{
	// TODO
}

void DataRemovedFromSelectedSubjectHandler(long iid)
{
	// TODO
}

void MitkViewer::SelectedDataChangedHandler(long iid)
{
	// This means that a different image is now in focus
	// but the subject didn't change! Thus, there is no need
	// for loading/unloading. Maybe for each iid
	// keep a reference to the node that holds it
	// and bring it forward or something

	// TODO
}

void MitkViewer::DataCheckedStateChangedHandler(long iid, bool checkState) 
{
	// An image got checked/unchecked in the viewer
	// Not really a priority I think
}

void SaveImageToFile(long iid)
{
	// Since the image has an iid, info about it
	// including the full path to it can be obtained
	// from the DataManager (already set up by ImageViewerBase).

	// If you need to create a new image (probably from the drawing tool?)
	// then show a dialog to get the filepath from the user,
	// and from a reference to the DataManager, call
	// long DataManager::AddDataToSubject(long uid, QString path, QString specialRole = QString(), 
	//	                                  QString type = QString(), QString name = QString()); 
	// uid is the subject id. 
	// path is... the path
	// specialRole should be "Mask" for a mask.
	// name should probably be "<Mask>" or something
	// type doesn't do anything for now
	// This will automatically update the tree and everything

	QString fullPath = m_DataManager->GetDataPath(iid);

	// TODO: Actually write the image 
}

MitkViewer::MitkViewer(QWidget *parent) : ImageViewerBase(parent)
{
	// Create an instance of QmitkStdMultiWidget and show it
	m_MitkWidget = new QmitkStdMultiWidget(this);
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(m_MitkWidget, 0, 0);
	this->setLayout(layout);

	// MITK Initialization
	m_MitkWidget->InitializeWidget();
	m_DataStorage = m_DataStorage = mitk::StandaloneDataStorage::New();
	m_MitkWidget->SetDataStorage(m_DataStorage);
	m_MitkWidget->InitPositionTracking();
	m_MitkWidget->EnablePositionTracking();
	auto geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);

	// Initialize bottom-right view as 3D view
	m_MitkWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

	// Enable standard handler for levelwindow-slider
	m_MitkWidget->EnableStandardLevelWindow();

	// Add the displayed views to the DataStorage to see their positions in 2D and 3D
	m_MitkWidget->AddDisplayPlaneSubTree();
	m_MitkWidget->AddPlanesToDataStorage();
	m_MitkWidget->SetWidgetPlanesVisibility(true);
}