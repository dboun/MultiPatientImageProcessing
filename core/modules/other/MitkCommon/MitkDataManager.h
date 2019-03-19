#ifndef MITK_DATA_MANAGER_H
#define MITK_DATA_MANAGER_H

#include <QObject>

#include <mitkDataNode.h>
#include <mitkStandaloneDataStorage.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <map>

#include "DataManager.h"
#include "DataViewBase.h"

class MitkDataManager : public DataManager
{
    Q_OBJECT

public:
    explicit MitkDataManager(QObject *parent = nullptr);
    virtual ~MitkDataManager();

    mitk::DataNode::Pointer GetMitkDataNode(long iid);

    // Don't delete a node directly from this, use RemoveSubject
    mitk::StandaloneDataStorage::Pointer GetMitkDataStorage();

public slots:
    /** Creates nodes for this subject (if they don't already exist).
     *  If it creates them it does so with reading files from the filesystem
    @param uid the subject id
    @return the request id. When the subject is loaded (or if it is already loaded)
            then MitkSubjectReady(long uid, long rid) will be emitted, where
            rid is the request id
    */
    long LoadSubjectIfNecessary(long uid);

    // If necessary (meaning nothing else uses the subject now),
    // it orders to write the changes to file 
    // and deletes the nodes for this subject
    void UnloadSubjectIfNecessary(long uid);

    void AddDataNodeToSubject(long uid, mitk::DataNode::Pointer dataNode,
        QString path, QString specialRole = QString(), 
        QString type = QString(), QString name = QString()
    );

    // Methods overriden from DataManager

    virtual void RemoveSubject(long uid) override;
    virtual long AddDataToSubject(long uid, QString path, 
		QString specialRole = QString(), QString type = QString(), 
        QString name = QString()
    ) override;
    virtual void RemoveData(long iid, bool silent = false) override;

signals:
    void MitkSubjectReady(long uid, long rid);

protected:
    std::map< long, std::atomic<int> >        m_MitkSubjectReferences;
    std::map< long, mitk::DataNode::Pointer > m_MitkDataNodes;

private:
    std::mutex                                m_MitkLoadMutex;
    mitk::StandaloneDataStorage::Pointer      m_DataStorage;
};

#endif // ! MITK_DATA_MANAGER_H