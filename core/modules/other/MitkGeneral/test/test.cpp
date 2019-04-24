#include <QTest>
#include <QDebug>
#include <QCoreApplication>
#include <set>
#include "DataManager.h"
#include "CustomMitkDataStorage.h"

class Test : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() { // Called once at the start
        m_DataManager = new DataManager(this);
        ds = CustomMitkDataStorage::CreateInstance(m_DataManager);
        connect(ds, SIGNAL(MitkLoadedNewNode(long, mitk::DataNode::Pointer)),
            (Test*)this, SLOT(OnMitkLoadedNewNode(long, mitk::DataNode::Pointer))
        );
    }
    void init() { // Called before each test case
        ds->SelectedSubjectChangedHandler(-1);
        for (const long& uid : m_DataManager->GetAllSubjectIds())
        {
            m_DataManager->RemoveSubject(uid);
        }
        m_LoadedDataIDs.clear();
    }
    void cleanupTestCase() { // Called once at the end
        //delete ds;
    }

    void loading2subjects() {

        long uid1 = m_DataManager->AddSubject(m_Path, "s1");
        long iid1_1 = m_DataManager->AddDataToSubject(uid1, m_ImagePath1, "", "Image");
        
        long uid2 = m_DataManager->AddSubject(m_Path, "s2");
        long iid2_1 = m_DataManager->AddDataToSubject(uid2, m_ImagePath1, "", "Image");
        long iid2_2 = m_DataManager->AddDataToSubject(uid2, m_ImagePath1, "", "Image");

        ds->SelectedSubjectChangedHandler(uid2);
        
        QVERIFY(loadedContains(iid2_1));
        QVERIFY(loadedContains(iid2_2));
        QCOMPARE(m_LoadedDataIDs.size(), 2);
    }

    void addSegmentations() {
        long uid1 = m_DataManager->AddSubject(m_Path, "s1");
        long iid1_1 = m_DataManager->AddDataToSubject(uid1, m_ImagePath2, 
            "Segmentation", "LabelSetImage"
        );
        long iid1_2 = m_DataManager->AddDataToSubject(uid1, m_ImagePath2, 
            "Segmentation", "LabelSetImage"
        );

        ds->SelectedSubjectChangedHandler(uid1);
        
        QCOMPARE(m_LoadedDataIDs.size(), 2);
    }

    void addSeeds() {
        long uid1 = m_DataManager->AddSubject(m_Path, "s1");
        long iid1_1 = m_DataManager->AddDataToSubject(uid1, m_ImagePath2, 
            "Seeds", "LabelSetImage"
        );
        long iid1_2 = m_DataManager->AddDataToSubject(uid1, m_ImagePath2, 
            "Seeds", "LabelSetImage"
        );

        ds->SelectedSubjectChangedHandler(uid1);
        
        QCOMPARE(m_LoadedDataIDs.size(), 2);
    }

public slots:
    void OnMitkLoadedNewNode(long iid, mitk::DataNode::Pointer dataNode)
    {
        qDebug() << "Testing: Loaded new Node";
        m_LoadedDataIDs.insert(iid);
    }

private:
    bool loadedContains(long iid)
    {
        return m_LoadedDataIDs.find(iid) != m_LoadedDataIDs.end();
    }

    CustomMitkDataStorage* ds;
    DataManager*           m_DataManager;
    QString                m_Path = QCoreApplication::applicationDirPath();
    QString                m_ImagePath1 = m_Path + "/test_image.nii.gz";
    QString                m_ImagePath2 = m_Path + "/test_image.nrrd";

    std::set<long>          m_LoadedDataIDs;
};

QTEST_MAIN(Test)
#include "test.moc"
