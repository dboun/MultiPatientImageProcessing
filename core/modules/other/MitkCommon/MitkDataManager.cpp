#include "MitkDataManager.h"

MitkDataManager::MitkDataManager(QObject *parent) : DataManager(parent)
{
    m_DataStorage = mitk::StandaloneDataStorage::New();
}

MitkDataManager::~MitkDataManager()
{

}

mitk::StandaloneDataStorage::Pointer MitkDataManager::GetMitkDataStorage()
{
    return m_DataStorage;
}