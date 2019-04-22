#include "CustomMitkDataStorage.h"

#include <QDebug>

#include <mitkIOUtil.h>

#include <stdexcept>
#include <string>

CustomMitkDataStorage& CustomMitkDataStorage::CreateInstance(DataManager* dataManager)
{
    m_DataManager = dataManager;
    m_CurrentSubjectID = -1;
    return GetInstance();
}

CustomMitkDataStorage& CustomMitkDataStorage::GetInstance()
{
    static CustomMitkDataStorage instance; // static is initialized only once
    return instance;
}

CustomMitkDataStorage::~CustomMitkDataStorage()
{
    
}

void CustomMitkDataStorage::SetDataView(DataViewBase* dataView)
{
    connect(dataView, SIGNAL(SelectedSubjectChanged(long)), 
        this, SLOT(SelectedSubjectChangedHandler(long))
    );
    connect(dataView, SIGNAL(SelectedDataChanged(long)), 
        this, SLOT(SelectedDataChangedHandler(long))
    );
	connect(dataView, SIGNAL(DataAddedForSelectedSubject(long)), 
        this, SLOT(DataAddedForSelectedSubjectHandler(long))
    );
	connect(dataView, SIGNAL(DataRemovedFromSelectedSubject(long)), 
        this, SLOT(DataRemovedFromSelectedSubjectHandler(long))
    );
	connect(dataView, SIGNAL(DataRequestedAsMask(long)), 
        this, SLOT(DataRequestedAsMaskHandler(long))
    );
	connect(dataView, SIGNAL(DataRequestedAsSegmentation(long)), 
        this, SLOT(DataRequestedAsSegmentationHandler(long))
    );
	connect(dataView, SIGNAL(ExportData(long, QString)),
        this, SLOT(ExportDataHandler(long, QString))
    );
}

long CustomMitkDataStorage::AddMitkImageToSubject(long uid, 
    mitk::Image::Pointer mitkImage, QString path, 
    QString specialRole, QString type, QString name,
    bool external, bool visibleInDataView)
{
    qDebug() << "CustomMitkDataStorage::AddImageToSubject";

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitkImage);
    
    return this->AddMitkNodeToSubject(uid, node, 
        path, specialRole, type, name, external, visibleInDataView
    );
}

long CustomMitkDataStorage::AddMitkLabelSetImageToSubject(long uid, 
    mitk::LabelSetImage::Pointer mitkImage, QString path,
    QString specialRole, QString type, QString name,
    bool external, bool visibleInDataView)
{
    qDebug() << "CustomMitkDataStorage::AddLabelSetImageToSubject";
    
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitkImage);
    
    return this->AddMitkNodeToSubject(uid, node, 
        path, specialRole, type, name, external, visibleInDataView
    );
}

long AddEmptyMitkImageToSubject(long uid, 
    QString specialRole, QString type, QString name,
    bool external, bool visibleInDataView)
{
    // TODO:
    return -1; // delete this
}

mitk::Image::Pointer CustomMitkDataStorage::GetImage(long iid)
{
    qDebug() << "CustomMitkDataStorage::GetImage" << iid;

    if (m_DataManager->GetSubjectIdFromDataId(iid) == -1)
    {
        throw std::invalid_argument( 
            std::string("CustomMitkDataStorage::GetImage: No such image, iid=") + 
            std::to_string(iid)
        );
    }

    if (m_DataManager->GetDataType(iid) != "Image")
    {
        throw std::invalid_argument( 
            std::string("CustomMitkDataStorage::GetImage: Not an image, iid=") + 
            std::to_string(iid)
        );
    }

    // TODO:
}

void CustomMitkDataStorage::WriteChangesToFileIfNecessaryForAllImagesOfCurrentSubject()
{
    if (m_CurrentSubjectID == -1) { return; }

    QRegExp numberRegExp("\\d*");  // a digit (\d), zero or more times (*)

    mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) 
    {
		QString nodeName = QString(it->Value()->GetName().c_str());
				
		// If the node name can be converted to a number 
		// (all of our node are named with their iid)
		if (numberRegExp.exactMatch(nodeName))
		{
			this->WriteChangesToFileIfNecessary(it.Value());
		}
	}
}

long CustomMitkDataStorage::AddMitkNodeToSubject(long uid, mitk::DataNode::Pointer dataNode, 
    QString path, QString specialRole, QString type, QString name,
    bool external, bool visibleInDataView )
{
    if (uid == m_CurrentSubjectID)
    {
        // TODO: Add node to m_NodesWaitMap. Add data to DataManager. 
        //       Handle DataAddedForSelectedSubject(long iid) to load it from m_NodesWaitMap
    }
    else {
        mitk::IOUtil::Save(
		    dataNode->GetData(), 
		    path.toStdString()
	    );
        
        m_DataManager->AddDataToSubject(uid, 
            path, specialRole, type, name, external, visibleInDataView
        );
    }
}

void CustomMitkDataStorage::SelectedSubjectChangedHandler(long uid)
{
	qDebug() << QString("CustomMitkDataStorage::SelectedSubjectChangedHandler()") << uid;
    m_CurrentSubjectID = uid;

	// Remove the previous ones
	QRegExp numberRegExp("\\d*");  // a digit (\d), zero or more times (*)
	
	mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
	for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it) 
    {
		QString nodeName = QString(it->Value()->GetName().c_str());
				
		// If the node name can be converted to a number 
		// (all of our node are named with their iid)
		if (numberRegExp.exactMatch(nodeName))
		{
			qDebug() << "Removing node with name: " << it->Value()->GetName().c_str();
			this->WriteChangesToFileIfNecessary(it.Value());
            //emit MitkNodeAboutToBeDeleted(std::stol(it.Value()->GetName().c_str()));
			this->Remove(it.Value());
		}
	}

	if (uid != -1) 
	{ 
		auto iids = m_DataManager->GetAllDataIdsOfSubject(uid);

		for(const long& iid : iids)
		{
			this->AddToDataStorage(iid);
		} 
	}
}

void CustomMitkDataStorage::DataAddedForSelectedSubjectHandler(long iid)
{
    this->AddToDataStorage(iid);
}

void CustomMitkDataStorage::DataRemovedFromSelectedSubjectHandler(long iid)
{
    auto node = this->GetNamedNode(QString::number(iid).toStdString().c_str());
	//this->WriteChangesToFileIfNecessary(node);
    this->Remove(node);
}

void CustomMitkDataStorage::DataRequestedAsMaskHandler(long iid)
{
    // TODO:
}

void CustomMitkDataStorage::DataRequestedAsSegmentationHandler(long iid)
{
    // TODO:
}

void CustomMitkDataStorage::ExportDataHandler(long iid, QString fileName)
{
    // TODO: Maybe this should handle showing the window
    // This could happen by having the default value "" as fileName
    // and showing the window if that's the case
    mitk::DataNode::Pointer dataNode = this->GetNamedNode(std::to_string(iid));

    if (dataNode)
    {
        mitk::IOUtil::Save(
		    dataNode->GetData(), 
		    fileName.toStdString()
	    );
    }
}

void CustomMitkDataStorage::AddToDataStorage(long iid)
{
    if (m_DataManager->GetDataType(iid) != "Image") { return; }
	
	QString specialRole = m_DataManager->GetDataSpecialRole(iid);
	QString dataPath    = m_DataManager->GetDataPath(iid);

    // "Mask" and "Segmentation" should be nrrd
	if ((specialRole == "Mask" || specialRole == "Segmentation") &&
		!dataPath.endsWith(".nrrd", Qt::CaseSensitive)
	) {
        qDebug() << "CustomMitkDataStorage: \"Mask\" or \"Segmentation\" was not nrrd";
		return;
	}

	qDebug() << "CustomMitkDataStorage::AddToDataStorage: Adding iid" << iid;
	QString dataName = m_DataManager->GetDataName(iid);

    // See if this is the result of waiting for the DataManager to update
    // when the node exists
    mitk::DataNode::Pointer dataNode;
    for (const long& tIid : IdsOfMap<long, mitk::DataNode::Pointer>(m_NodesWaitMap))
    {
        if (tIid == iid)
        {
            dataNode = m_NodesWaitMap[tIid];
            m_NodesWaitMap.erase(tIid);
            break;
        }
    }

    if (!dataNode)
    {
        // The dataNode doesn't already exist
        mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(
		    dataPath.toStdString(), *this
	    );
        if (dataNodes.IsNull()) { return; }
	    dataNode = dataNodes->at(0);
    }

	dataNode->SetName(QString::number(iid).toStdString().c_str());
    //dataNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));
	dataNode->SetVisibility(false);

    //dataNode->SetProperty("fixedLayer", mitk::BoolProperty::New(true));
    //dataNode->SetProperty("layer", mitk::IntProperty::New(2));

	if (specialRole == QString("Mask"))
	{
        // dataNode->SetProperty("fixedLayer", mitk::BoolProperty::New(true));
        // dataNode->SetProperty("layer", mitk::IntProperty::New(48));

        //emit MitkLoadedNewMask(dataNode);
	}

	if (specialRole == QString("Segmentation"))
	{
	    // auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());
	    // labelSetImage->GetActiveLabelSet()->SetActiveLabel(0);
	}
	
    emit MitkLoadedNewNode(iid, dataNode);
}

void CustomMitkDataStorage::WriteChangesToFileIfNecessary(mitk::DataNode::Pointer dataNode)
{
    if (!dataNode) { return; }
    long iid = QString(dataNode->GetName().c_str()).toLong();

    if (m_DataManager->GetDataIsExternal(iid) || m_DataManager->GetDataType(iid) != "Image") 
    { 
        return; 
    }

    QString specialRole = m_DataManager->GetDataSpecialRole(iid);

    if (specialRole == "Mask" || specialRole == "Segmentation")
    {
        mitk::IOUtil::Save(
		    dataNode->GetData(), 
		    m_DataManager->GetDataPath(iid).toStdString()
	    );
    }
}

DataManager* CustomMitkDataStorage::m_DataManager;

long         CustomMitkDataStorage::m_CurrentSubjectID;

std::map<long, mitk::DataNode::Pointer> CustomMitkDataStorage::m_NodesWaitMap;