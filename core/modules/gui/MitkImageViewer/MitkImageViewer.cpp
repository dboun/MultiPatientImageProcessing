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

MitkImageViewer::MitkImageViewer(QWidget *parent) : ImageViewerBase(parent)
{
	// Create an instance of CustomQmitkStdMultiWidget and show it
	m_MitkWidget = new CustomQmitkStdMultiWidget(this);
	m_MitkWidget->AddSlidersToViews();
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(m_MitkWidget, 0, 0);
	this->setLayout(layout);

	// MITK Initialization
	m_MitkWidget->InitializeWidget();
	//m_DataStorage = mitk::StandaloneDataStorage::New();
	m_DataStorage = &CustomMitkDataStorage::GetInstance();
	m_MitkWidget->SetDataStorage(m_DataStorage);
	m_MitkWidget->InitPositionTracking();
	m_MitkWidget->EnablePositionTracking();
	auto geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);
	mitk::Color color; 
	//color = 0.57f, 0.57f, 0.57f;
	//color = 0.05f, 0.10f, 0.15f;
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

	if (iid == -1) { return; }

	qDebug() << "Changing opacity of " << iid <<" to value " << value;

	mitk::DataNode* node = m_DataStorage->GetNamedNode(
		QString::number(iid).toStdString().c_str()
	);

	if (node)
	{
		//node->SetProperty("layer", mitk::IntProperty::New(1));
		node->SetProperty("opacity", mitk::FloatProperty::New(value/100.0));
		//node->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

MitkImageViewer::~MitkImageViewer()
{

}

void MitkImageViewer::SelectedSubjectChangedHandler(long uid) 
{
	m_MitkWidget->ResetCrosshair();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	m_FirstTimeForThisSubject = true;
}

void MitkImageViewer::DataRemovedFromSelectedSubjectHandler(long iid)
{
	m_MitkWidget->RequestUpdate();
}

void MitkImageViewer::SelectedDataChangedHandler(long iid)
{

}

void MitkImageViewer::DataCheckedStateChangedHandler(long iid, bool checkState) 
{
	// An image got checked/unchecked in the viewer
	// All the images of a new selected subject start unchecked
	qDebug() << QString("MitkImageViewer::DataCheckedStateChangedHandler()") 
		<< iid << "( Special Role:" << this->GetDataManager()->GetDataSpecialRole(iid) << ")";

	mitk::DataNode::Pointer dataNode = m_DataStorage->GetNamedNode(
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
	if (m_FirstTimeForThisSubject)
	{
		m_MitkWidget->ResetCrosshair();
		m_FirstTimeForThisSubject = false;
	}

	dataNode->GetData()->Modified();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void MitkImageViewer::OnExportData(long iid, QString fileName)
{
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
	
	// Parent dir of file
	QString filePathTemp = fileName;
	filePathTemp.replace("\\", "/", Qt::CaseSensitive);
	QStringList filePathSplit = filePathTemp.split("/");
	QString parentDirOfFile = filePathSplit.value(filePathSplit.length() - 2);
	
	qDebug() << "Parent dir of file" << parentDirOfFile;

	// The name of the file
	QFileInfo f(fileName);
	QString baseName = f.baseName();

	// Create output directory if it doesn't exist
	if (!QDir(parentDirOfFile).exists())
	{
		if (!QDir().mkpath(parentDirOfFile)) { 
			QMessageBox::warning(this, 
				"Problem exporting",
				"No permission to write in this directory"
			);
			return; 
		}
	}

	qDebug() << "Will save image to:" << fileName;

	mitk::IOUtil::Save(
		dataNode->GetData(), 
		fileName.toStdString()
	);
}

void MitkImageViewer::SaveImageToFile(long iid, bool updateDataManager)
{
	qDebug() << "MitkImageViewer::SaveImageToFile" << iid;

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
		qDebug() << "MitkImageViewer::SaveImageToFile error: Can't find dataNode" << iid;
		return;
	}

	long uid = this->GetDataManager()->GetSubjectIdFromDataId(iid);
	
	// QFileInfo f(this->GetDataManager()->GetDataPath(iid));
	// QString baseName = f.baseName();

	// QString specialRole   = this->GetDataManager()->GetDataSpecialRole(iid);
	// QString directoryName = this->GetDataManager()->GetSubjectPath(uid);

	// if (specialRole != "")
	// {
	// 	directoryName += QString("/") + m_AppNameShort + QString("/")
	// 	               + m_AppNameShort + "_" + specialRole;
	// }

	// if (!QDir(directoryName).exists())
	// {
	// 	QDir().mkpath(directoryName);
	// }

	// QString nifti = directoryName + QString("/") + baseName + QString(".nii.gz");
	// QString nrrd  = directoryName + QString("/") + baseName + QString(".nrrd");

	// qDebug() << "Will save image to: ";
	// qDebug() << "-  " << nifti;
	// qDebug() << "-  " << nrrd;

	// qDebug() << "MitkImageViewer saving image" << nifti;
	// qDebug() << "MitkImageViewer saving image" << nrrd;


	QString specialRole = this->GetDataManager()->GetDataSpecialRole(iid);
	QString path = this->GetDataManager()->GetDataPath(iid);
	qDebug() << "MitkImageViewer where" << path << "was";

	mitk::IOUtil::Save(
		dataNode->GetData(), 
		// nifti.toStdString()
		path.toStdString()
	);
	
	// if (specialRole == "Mask" || specialRole == "Segmentation")
	// {
	// 	mitk::IOUtil::Save(
	// 		dataNode->GetData(), 
	// 		nrrd.toStdString()
	// 	);
	// }

	if (updateDataManager)
	{
		// auto iids = this->GetDataManager()->GetAllDataIdsOfSubject(
		// 	this->GetDataManager()->GetSubjectIdFromDataId(iid)
		// );

		// if (specialRole == "Mask")
		// {
		// 	for (const long& tIid : iids)
		// 	{
		// 		if (this->GetDataManager()->GetDataSpecialRole(tIid) == "Mask")
		// 		{
		// 			this->GetDataManager()->RemoveData(tIid, true);
		// 		}
		// 	}
		// }
		this->GetDataManager()->RemoveData(iid, true);
		this->GetDataManager()->AddDataToSubject(
			uid, path, specialRole, "Image"
		);

		// this->GetDataManager()->AddDataToSubject(
		// 	uid, nifti, specialRole
		// );

		// this->GetDataManager()->AddDataToSubject(
		// 	uid, nrrd, specialRole
		// );
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
	mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>(
		imagePath.toStdString()
	);

	mitk::Image::Pointer image;
	if (inputImage->GetDimension() == 3)
	{
		image = inputImage;
	}
	else {
		mitk::Convert2Dto3DImageFilter::Pointer convertFilter = mitk::Convert2Dto3DImageFilter::New();
		convertFilter->SetInput(inputImage);
		convertFilter->Update();
		image = convertFilter->GetOutput();
	}

	// The output image
	mitk::LabelSetImage::Pointer outputImage = mitk::LabelSetImage::New();
	
	// Copy the data from input image
	try {
		outputImage->InitializeByLabeledImage(image);
	}
	catch (mitk::Exception e)
	{
		qDebug() << "Initializing didn't work";
		qDebug() << e.GetDescription();
	}
	
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
			uid, outputImagePath, imageSpecialRole, "Image"
		);
	}
}