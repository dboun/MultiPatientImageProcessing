#ifndef MITK_DATA_MANAGER_H
#define MITK_DATA_MANAGER_H

#include <QObject>

#include <mitkDataNode.h>
#include <mitkStandaloneDataStorage.h>

#include <atomic>
#include <memory>

#include "DataManager.h"

class MitkDataManager : public DataManager
{
    Q_OBJECT

public:
    explicit MitkDataManager(QObject *parent = nullptr);
    virtual ~MitkDataManager();

    typedef struct : Data {
		mitk::DataNode::Pointer dataNode;
        std::atomic<int> references = {0};
	} MitkData;

	typedef struct : Subject {
        bool isLoaded = false;
		mitk::DataNode::Pointer subjectNode;
	} MitkSubject;

    bool IsSubjectLoadedInStorage(long uid);

    mitk::DataNode::Pointer GetMitkSubjectNode(long uid);
    mitk::DataNode::Pointer GetMitkDataNode(long iid);

    // Creates nodes for this subject
    void LoadSubjectIfNecessary(long uid);

    // Orders to write the changes to file and deletes the nodes for this subject
    void UnloadSubjectIfNecessary(long uid);

    virtual mitk::StandaloneDataStorage::Pointer GetMitkDataStorage();

private:

    mitk::StandaloneDataStorage::Pointer m_DataStorage;
};

#endif // ! MITK_DATA_MANAGER_H