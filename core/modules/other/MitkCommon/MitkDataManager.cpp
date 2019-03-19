#include "MitkDataManager.h"

MitkDataManager::MitkDataManager(QObject *parent) : DataManager(parent)
{
    m_DataStorage = mitk::StandaloneDataStorage::New();
}

MitkDataManager::~MitkDataManager()
{

}

void MitkDataManager::LoadSubjectIfNecessary(long uid)
{

}

void MitkDataManager::UnloadSubjectIfNecessary(long uid)
{

}

mitk::DataNode::Pointer MitkDataManager::GetMitkDataNode(long iid)
{
    if (m_MitkDataNodes.count(iid))
    {
        return m_MitkDataNodes[iid];
    }
    return nullptr;
}

mitk::StandaloneDataStorage::Pointer MitkDataManager::GetMitkDataStorage()
{
    return m_DataStorage;
}

void MitkDataManager::RemoveSubject(long uid)
{
    // TODO Save changes
}

long MitkDataManager::AddDataToSubject(long uid, QString path = QString(), 
    QString specialRole = QString(), QString type = QString(), 
    QString name = QString())
{
    
}

void MitkDataManager::RemoveData(long iid, bool silent = false)
{
    // TODO

    DataManager::RemoveData(iid, silent);
}