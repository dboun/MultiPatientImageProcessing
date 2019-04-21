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

long CustomMitkDataStorage::AddMitkImageToSubject(long uid, mitk::Image::Pointer mitkImage, 
    QString specialRole, QString type, QString name,
    bool external, bool visibleInDataView)
{
    qDebug() << "CustomMitkDataStorage::AddImageToSubject";
    
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

void CustomMitkDataStorage::DataAboutToGetRemovedHandler(long iid)
{
    qDebug() << "CustomMitkDataStorage::DataAboutToGetRemovedHandler" << iid;
    
    if (m_CurrentSubjectID != m_DataManager->GetSubjectIdFromDataId(iid))
    {
        return;
    }

    if (m_DataManager->GetDataSpecialRole(iid) == "" ||
        m_DataManager->GetDataIsExternal(iid)  == true)
    {
        return;
    }
    
    // TODO:
}

void CustomMitkDataStorage::SelectedSubjectChangedHandler(long uid)
{
    if (m_CurrentSubjectID != -1)
    {
        // TODO: empty DataStorage
    }

    m_CurrentSubjectID = uid;

    if (uid == -1)
    {
        return;
    }

    // TODO: add new images to DataStorage
}

void CustomMitkDataStorage::DataAddedForSelectedSubjectHandler(long iid)
{
    // TODO:
}

void CustomMitkDataStorage::DataRemovedFromSelectedSubjectHandler(long iid)
{
    // TODO:
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
    mitk::DataNode::Pointer dataNode = this->GetNamedNode(std::to_string(iid));

    mitk::IOUtil::Save(
		dataNode->GetData(), 
		fileName.toStdString()
	);

    // TODO:
}