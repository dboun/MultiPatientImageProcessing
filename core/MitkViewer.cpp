#include "MitkViewer.h"

#include <QDebug>
#include <QMouseEvent>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

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

void MitkViewer::OpacitySliderHandler(int value) 
{
	// We should probably leave this for last
}

void MitkViewer::SelectedSubjectChangedHandler(long uid) 
{
    qDebug() << QString("MitkViewer::SelectedSubjectChangedHandler()") << uid;
    QString fullDataPath = m_DataManager->GetDataPath(uid);
    QString fullSubjectPath = m_DataManager->GetSubjectPath(uid);

    qDebug() << " data path = " << fullDataPath << "\n";
    qDebug() << " sub path = " << fullSubjectPath << "\n";
	// This means that a *new* subject is selected

	// TODO: Destroy everything that is loaded

	// Whenever a new subject is selected
	// All its images are unchecked so there is pretty much
	// nothing more to do here
}

void MitkViewer::DataAddedForSelectedSubjectHandler(long iid)
{
	// TODO: Leave it for now, the gui doesn't support
	// putting new images to an existing subject yet
}

void MitkViewer::DataRemovedFromSelectedSubjectHandler(long iid)
{
	// TODO: Leave for last 
}

void MitkViewer::SelectedDataChangedHandler(long iid)
{

    qDebug() << QString("MitkViewer::SelectedDataChangedHandler()") << iid;

    QString imagePath = m_DataManager->GetDataPath(iid);

    // Remove the previous ones
    if (!lastImagePath.isEmpty()) {
        m_DataStorage->Remove(
            m_DataStorage->GetNamedNode( lastImagePath.toStdString().c_str() )
        );
    }

    lastImagePath.clear();

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
            emit DisplayedDataName(imagePath);
        }

        lastImagePath = imagePath;
    }

    m_MitkWidget->ResetCrosshair();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    // This means that a different image is now in focus
	// but the subject didn't change! Thus, there is no need
	// for loading/unloading.
	
	// Not sure what exactly what the correct thing to do is
	// Maybe for each iid
	// keep a reference to the node that holds it
	// and bring it forward or something.

	// TODO
}

void MitkViewer::DataCheckedStateChangedHandler(long iid, bool checkState) 
{
	// An image got checked/unchecked in the viewer

	// All the images of a new selected subject start unchecked
	// so here we add them or remove them to the data storage

	// TODO
}

void MitkViewer::SaveImageToFile(long iid)
{
	// Since the image has an iid, info about it
	// including the full path to it can be obtained
	// from the DataManager (already set up by ImageViewerBase).
	QString fullPath = m_DataManager->GetDataPath(iid);

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

	// TODO: Actually write the image 
}
