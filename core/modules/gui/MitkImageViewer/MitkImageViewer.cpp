#include "MitkImageViewer.h"

#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDir>

#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

#include <iostream>

MitkImageViewer::MitkImageViewer(QWidget *parent) : ImageViewerBase(parent)
{
	// Create an instance of QmitkStdMultiWidget and show it
	m_MitkWidget = new QmitkStdMultiWidget(this);
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(m_MitkWidget, 0, 0);
	this->setLayout(layout);

	// MITK Initialization
	m_MitkWidget->InitializeWidget();
	m_DataStorage = mitk::StandaloneDataStorage::New();
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

void MitkImageViewer::OpacitySliderHandler(int value) 
{
	// We should probably leave this for last
  //QString name = m_DataManager->GetDataName(value);
  
  long iid = this->GetDataView()->GetCurrentDataID();

  if (iid == -1) { return; }
	
  qDebug() << "Changing opacity of " << iid <<" to value " << value;

  mitk::DataNode* node = m_DataStorage->GetNamedNode(
	  QString::number(iid).toStdString().c_str()
  );
  //node->SetProperty("layer", mitk::IntProperty::New(1));
  node->SetProperty("opacity", mitk::FloatProperty::New(value/100.0));
  //node->GetData()->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

MitkImageViewer::~MitkImageViewer()
{

}

void MitkImageViewer::SelectedSubjectChangedHandler(long uid) 
{
	// This means that a new subject is selected

	qDebug() << QString("MitkImageViewer::SelectedSubjectChangedHandler()") << uid;

	// Remove the previous ones
	QRegExp numberRegExp("\\d*");  // a digit (\d), zero or more times (*)
	
	mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) {
		qDebug() << "Removing node with name: " << it->Value()->GetName().c_str();
		
		QString nodeName = QString(it->Value()->GetName().c_str());
				
		// If the node name can be converted to a number 
		// (all of our node are named with their iid)
		if (numberRegExp.exactMatch(nodeName))
		{
			m_DataStorage->Remove(it.Value());
		}
	}

	auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(uid);

	for(const long& iid : iids)
	{
		this->AddToDataStorage(iid);
	}

	m_MitkWidget->ResetCrosshair();
	//mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void MitkImageViewer::DataAddedForSelectedSubjectHandler(long iid)
{
	AddToDataStorage(iid);
}

void MitkImageViewer::DataRemovedFromSelectedSubjectHandler(long iid)
{
	qDebug() << "MitkImageViewer::DataRemovedFromSelectedSubjectHandler";
	
	m_DataStorage->Remove(
		m_DataStorage->GetNamedNode(QString::number(iid).toStdString().c_str())
	);
}

void MitkImageViewer::SelectedDataChangedHandler(long iid)
{
 //   qDebug() << QString("MitkImageViewer::SelectedDataChangedHandler()") << iid;

 //   QString imagePath = m_DataManager->GetDataPath(iid);

 //   // Remove the previous ones
 //   if (!lastImagePath.isEmpty()) {
 //       m_DataStorage->Remove(
 //           m_DataStorage->GetNamedNode( lastImagePath.toStdString().c_str() )
 //       );
 //   }

 //   lastImagePath.clear();

 //   // Load datanode (eg. many image formats, surface formats, etc.)
 //   if (imagePath.toStdString() != "")
 //   {
 //       qDebug() << QString("MPIP: Trying to display image...");
 //       mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(imagePath.toStdString(), *m_DataStorage);

 //       if (dataNodes->empty()) {
 //           qDebug() << QString("Could not open file: ") << imagePath;
 //       }
 //       else {
 //           //dataNodes->Modified();
 //           //m_DataStorage->Modified();
 //           //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
 //           mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());

 //           mitk::DataNode::Pointer newNode = mitk::DataNode::New();
 //           newNode->SetName(imagePath.toStdString().c_str());
 //           newNode->SetData(image);
 //           newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
 //           m_DataStorage->Add(newNode);
 //           emit DisplayedDataName(iid);
 //       }

 //       lastImagePath = imagePath;
 //   }

 //   m_MitkWidget->ResetCrosshair();
 //   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
 //   // This means that a different image is now in focus
	//// but the subject didn't change! Thus, there is no need
	//// for loading/unloading.
	//
	//// Not sure what exactly what the correct thing to do is
	//// Maybe for each iid
	//// keep a reference to the node that holds it
	//// and bring it forward or something.

	//// TODO
}

void MitkImageViewer::DataCheckedStateChangedHandler(long iid, bool checkState) 
{
  // An image got checked/unchecked in the viewer
  // All the images of a new selected subject start unchecked
  // so here we add them or remove them to the data storage
  qDebug() << QString("MitkImageViewer::DataCheckedStateChangedHandler()") 
		<< iid << "( Special Role:" << this->GetDataManager()->GetDataSpecialRole(iid) << ")";

	mitk::DataNode::Pointer dataNode = this->m_DataStorage->GetNamedNode(
		QString::number(iid).toStdString().c_str()
	);

	if (!dataNode)
	{
		qDebug() << "MitkImageViewer::DataCheckedStateChangedHandler() " << "dataNode not found";
		return;
	}	

	if (checkState)
	{
		//dataNode->SetVisibility(true);

		QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
		if (specialRole == "Segmentation")
		{
			dataNode->SetProperty("layer", mitk::IntProperty::New(3));
		}
		else if (specialRole == "Mask")
		{
			dataNode->SetProperty("layer", mitk::IntProperty::New(4));
		}
		else {
			dataNode->SetProperty("layer", mitk::IntProperty::New(2));
		}

		dataNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
	}
	else {
		//dataNode->SetVisibility(false);
		dataNode->SetProperty("layer", mitk::IntProperty::New(1));
		dataNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));
	}

  dataNode->SetVisibility(checkState);
	if (checkState)
	{
		//m_MitkWidget->ResetCrosshair();
	}

	dataNode->GetData()->Modified();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	//mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

  // if (checkState)
  // {

  //   // QString imageName;// don't actually use this -> = m_DataManager->GetDataName(iid);
  //   // QString imagePath = this->GetDataManager()->GetDataPath(iid);

	// 	// if (imagePath.isEmpty()) { return; }

	// 	// QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
	// 	// qDebug() << "Got here";

	// 	// // Load datanode (eg. many im			
	// 	// mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(imagePath.toStdString(), *m_DataStorage);

	// 		// if (dataNodes->empty()) {
	// 		// 	qDebug() << QString("Could not open file: ") << imagePath;
	// 		// 	delete dataNodes;
	// 		// 	return;
	// 		// }

	// 		// //dataNodes->Modified();
	// 		// //m_DataStorage->Modified();
	// 		// //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	// 		// mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());

	// 		// mitk::DataNode::Pointer newNode = mitk::DataNode::New();
	// 		// QFileInfo f(imagePath);
	// 		// qDebug() << "basename = " << f.baseName();
	// 		// imageName = f.baseName();
	// 		// qDebug() << "adding node with name = " << imageName;
	// 		// newNode->SetData(image);
	// 		// newNode->SetName(imageName.toStdString().c_str());
	// 		// newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
	// 		// m_DataStorage->Add(newNode);
	// 		// m_LoadedImages.push_back(iid);
	// 		// //emit DisplayedDataName(iid);age formats, surface formats, etc.)
	// 	// TODO: Maybe delete if?
	// 	if (specialRole == QString())
	// 	{
	// 		qDebug() << QString("MitkImageViewer: Trying to display image...");
	// 		// mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(imagePath.toStdString(), *m_DataStorage);

	// 		// if (dataNodes->empty()) {
	// 		// 	qDebug() << QString("Could not open file: ") << imagePath;
	// 		// 	delete dataNodes;
	// 		// 	return;
	// 		// }

	// 		// //dataNodes->Modified();
	// 		// //m_DataStorage->Modified();
	// 		// //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	// 		// mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());

	// 		// mitk::DataNode::Pointer newNode = mitk::DataNode::New();
	// 		// QFileInfo f(imagePath);
	// 		// qDebug() << "basename = " << f.baseName();
	// 		// imageName = f.baseName();
	// 		// qDebug() << "adding node with name = " << imageName;
	// 		// newNode->SetData(image);
	// 		// newNode->SetName(imageName.toStdString().c_str());
	// 		// newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
	// 		// m_DataStorage->Add(newNode);
	// 		// m_LoadedImages.push_back(iid);
	// 		// //emit DisplayedDataName(iid);
	// 	}
	// 	else {
	// 		//mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(imagePath.toStdString(), *m_DataStorage);
	// 		qDebug() << QString("MPIP: Trying to display labels image...");

	// 		//QString kindaRandomImage = QFileInfo(m_DataManager->GetDataPath(m_CurrentData)).baseName();
	// 		//mitk::DataNode::Pointer referenceNode = this->m_DataStorage->GetNamedNode(kindaRandomImage.toStdString().c_str());

	// 		//if (!referenceNode)
	// 		//{
	// 		//	QMessageBox::information(
	// 		//		this, "Show mask", "For now select at least one normal image before selecting a mask file.");
	// 		//	return;
	// 		//}

	// 		//mitk::LabelSetImage::Pointer labelsImage = mitk::IOUtil::Load<mitk::LabelSetImage>(
	// 		//	imagePath.toStdString()
	// 		//);

	// 		//mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
	// 		//workingNode->SetData(labelsImage);
	// 		//workingNode->SetName(imageName.toStdString().c_str());

	// 		//if (!this->m_DataStorage->Exists(workingNode))
	// 		//{
	// 		//	this->m_DataStorage->Add(workingNode, referenceNode);
	// 		//}

	// 		//mitk::DataNode::Pointer newNode = mitk::DataNode::New();
	// 		//QFileInfo f(imagePath);
	// 		//qDebug() << "basename = " << f.baseName();
	// 		//imageName = f.baseName();
	// 		//qDebug() << "adding node with name = " << imageName;
	// 		//newNode->SetData(labelsImage);
	// 		//newNode->SetName(imageName.toStdString().c_str());
	// 		////newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
	// 		//m_DataStorage->Add(newNode);
	// 		//m_LoadedImages.push_back(iid);
	// 		//emit DisplayedDataName(iid);
	// 	}

  //   m_MitkWidget->ResetCrosshair();
  //   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  //   this->m_CurrentData = iid;
  // }
  // else {
    // qDebug() << QString("inside else ");
    // QString name = m_DataManager->GetDataName(iid);
    // QString path = m_DataManager->GetDataPath(iid);
    // QFileInfo f(path);
    // f.baseName();
    // qDebug() << "basename = " << f.baseName();
    // name = f.baseName();
    
    // qDebug() << name;

    // mitk::DataNode* node = m_DataStorage->GetNamedNode(name.toStdString().c_str());
    // std::string nodename = node->GetName();
    // qDebug() << "nodename = " << QString(nodename.c_str());

    // mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
    // for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
    // {
    //   if (it.Value()->GetName() == name.toStdString())
    //   {
    //     m_DataStorage->Remove(it.Value());
    //   }
    //   else {
    //     qDebug() << "Not removing node with name: " << it.Value()->GetName().c_str();
    //   }
    // }
      

    // /*
    // m_DataStorage->Remove(node);*/
     
    // m_LoadedImages.removeAll(iid);
    // m_MitkWidget->ResetCrosshair();
    // mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    // mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  //}
}

void MitkImageViewer::SaveImageToFile(long iid, bool updateDataManager)
{
	// Since the image has an iid, info about it
	// including the full path to it can be obtained
	// from the DataManager (already set up by ImageViewerBase).
	qDebug() << "MitkImageViewer::SaveImageToFile";

	// Find the node
	mitk::DataNode::Pointer dataNode;

	mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) {
		if (QString::number(iid).toStdString() == it->Value()->GetName())
		{
			dataNode = it->Value();
			break;
		}
	}

	if (!m_DataStorage->Exists(dataNode))
	{
		qDebug() << "Can't find dataNode to save image " << iid;
		return;
	}

	long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);
	
	QFileInfo f(this->GetDataManager()->GetDataPath(iid));
	QString baseName = f.baseName();

	QString directoryName = this->GetDataManager()->GetSubjectPath(uid) 
		+ QString("/") + m_AppNameShort + QString("/")
		+ m_AppNameShort + "_" + "Mask";

	if (!QDir(directoryName).exists())
	{
		QDir().mkpath(directoryName);
	}

	QString nifti = directoryName + QString("/") + baseName + QString(".nii.gz");
	QString nrrd  = directoryName + QString("/") + baseName + QString(".nrrd");

	qDebug() << "Will save image to: ";
	qDebug() << "-  " << nifti;
	qDebug() << "-  " << nrrd;

	qDebug() << "MitkImageViewer saving image" << nifti;
	qDebug() << "MitkImageViewer saving image" << nrrd;
	qDebug() << "MitkImageViewer where" << this->GetDataManager()->GetDataPath(iid) << "was";


	QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
	
	mitk::IOUtil::Save(
		dataNode->GetData(), 
		nifti.toStdString()
	);
	
	if (specialRole == "Mask" || specialRole == "Segmentation")
	{
		mitk::IOUtil::Save(
			dataNode->GetData(), 
			nrrd.toStdString()
		);
	}

	if (updateDataManager)
	{
		auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(
			this->GetDataManager()->GetSubjectIdFromDataId(iid)
		);

		if (specialRole == "Mask")
		{
			for (const long& tIid : iids)
			{
				if (this->GetDataManager()->GetDataSpecialRole(tIid) == "Mask")
				{
					this->GetDataManager()->RemoveData(tIid, true);
				}
			}
		}


		this->GetDataManager()->AddDataToSubject(
			uid, nifti, specialRole
		);

		this->GetDataManager()->AddDataToSubject(
			uid, nrrd, specialRole
		);
	}
}

void MitkImageViewer::ConvertToNrrdAndSave(long iid, long referenceIid, bool updateDataManager)
{
	qDebug() << "MitkImageViewer::ConvertToNrrdAndSave" << iid << referenceIid;

	// Reference image info
	QString referencePath = this->GetDataManager()->GetDataPath(referenceIid);

	// Image info
	QString imageSpecialRole = this->GetDataManager()->GetDataSpecialRole(iid);
	QString imagePath = this->GetDataManager()->GetDataPath(iid);
	long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);

	// Load the two images
	mitk::LabelSetImage::Pointer referenceImage = mitk::IOUtil::Load<mitk::LabelSetImage>(
		referencePath.toStdString()
	);
	mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(
		imagePath.toStdString()
	);

	// The output image
	mitk::LabelSetImage::Pointer outputImage = mitk::LabelSetImage::New();
	
	// Copy the data from input image
	outputImage->InitializeByLabeledImage(image);
	
	// Copy the labels from reference image
	mitk::LabelSet::Pointer referenceLabelSet =	referenceImage->GetActiveLabelSet();
	mitk::LabelSet::Pointer outputLabelSet    =	outputImage->GetActiveLabelSet();

	mitk::LabelSet::LabelContainerConstIteratorType itR;
	mitk::LabelSet::LabelContainerConstIteratorType it;
	
	for (itR =  referenceLabelSet->IteratorConstBegin();
			 itR != referenceLabelSet->IteratorConstEnd(); 
			 ++itR) 
	{
		for (it = outputLabelSet->IteratorConstBegin(); 
				 it != outputLabelSet->IteratorConstEnd();
				 ++it)
		{
			if (itR->second->GetValue() == it->second->GetValue())
			{
				it->second->SetColor(itR->second->GetColor());
				it->second->SetName(itR->second->GetName());
			}
		}
	}
	
	// Where to save the new image
	QString directoryName = this->GetDataManager()->GetSubjectPath(uid) 
		+ QString("/") + m_AppNameShort + QString("/")
		+ m_AppNameShort + "_" + imageSpecialRole;

	if (!QDir(directoryName).exists())
	{
		QDir().mkpath(directoryName);
	}

	// Find the full path to save the output image
	QFileInfo f(imagePath);
	QString outputImagePath = directoryName + "/" + f.baseName() + ".nrrd";

	// Save the image to file
	qDebug() << "MitkImageViewer::ConvertToNrrdAndSave: Saving to file";
	mitk::IOUtil::Save(
		outputImage, outputImagePath.toStdString()
	);
	qDebug() << "MitkImageViewer::ConvertToNrrdAndSave: Saving to file finished";

	if (updateDataManager)
	{
		qDebug() << "MitkImageViewer::ConvertToNrrdAndSave: Updating data manager";
		this->GetDataManager()->AddDataToSubject(
			uid, outputImagePath, imageSpecialRole
		);
	}
}

void MitkImageViewer::AddToDataStorage(long iid)
{
	QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
	QString dataPath    = this->GetDataManager()->GetDataPath(iid);

	if ((specialRole == "Mask" || specialRole == "Segmentation") &&
		!dataPath.endsWith(".nrrd", Qt::CaseSensitive)
	) {
		return;
	}

	qDebug() << "MitkImageViewer: Adding iid" << iid;
	QString dataName = this->GetDataManager()->GetDataName(iid);

	mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(
		dataPath.toStdString(), *m_DataStorage
	);

	mitk::DataNode::Pointer dataNode = dataNodes->at(0);
	dataNode->SetName(QString::number(iid).toStdString().c_str());
    //dataNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));
    dataNode->SetVisibility(false);

//	dataNode->SetProperty("fixedLayer", mitk::BoolProperty::New(true));
//	dataNode->SetProperty("layer", mitk::IntProperty::New(2));

	if (specialRole == QString("Mask"))
	{
//		dataNode->SetProperty("fixedLayer", mitk::BoolProperty::New(true));
//		dataNode->SetProperty("layer", mitk::IntProperty::New(48));

		emit MitkLoadedNewMask(dataNode);
	}

	if (specialRole == QString("Segmentation"))
	{
		auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());
		labelSetImage->GetActiveLabelSet()->SetActiveLabel(0);
	}
}
