#ifndef OBSERVER_BASE_H
#define OBSERVER_BASE_H

#include <QObject>
#include <QString>

#include "DataManager.h"
#include "DataViewBase.h"

/** This class is meant to be used to observe the state and be the base class
 *  for classes that trigger actions based on the state */
class ObserverBase : public QObject
{
    Q_OBJECT

public:
    ObserverBase(QObject* parent = nullptr);

    ~ObserverBase();

    virtual void SetDataManager(DataManager* dataManager);

    virtual void SetDataView(DataViewBase* dataView);

    QString GetName(); 

public slots:
    // Slots for DataViewBase
	virtual void SelectedSubjectChangedHandler(long uid);
	virtual void SelectedDataChangedHandler(long iid);
	virtual void DataAddedForSelectedSubjectHandler(long iid);
	virtual void DataRemovedFromSelectedSubjectHandler(long iid);
	virtual void DataCheckedStateChangedHandler(long iid, bool checkState);

    // Slots for DataManager (Useful only if an implementation concerns all subjects)
	void SubjectAddedHandler(long uid);
	void SubjectRemovedHandler(long uid);
	void SubjectDataChangedHandler(long uid);

protected:
    // Implementations of this class, should call the below methods to update their state
    void SetName(QString name);

    DataManager*           m_DataManager;
    DataViewBase*          m_DataView;

private:
    QString                m_Name;
};

#endif // ! OBSERVER_BASE_H