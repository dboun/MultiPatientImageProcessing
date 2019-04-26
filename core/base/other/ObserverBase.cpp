#include "ObserverBase.h"

#include <QDebug>

ObserverBase::ObserverBase(QObject* parent) : 
    QObject(parent),
    m_Name("Observer") // This needs to be set this manually later
{

}

ObserverBase::~ObserverBase()
{

}

void ObserverBase::SetDataManager(DataManager* dataManager)
{
    m_DataManager = dataManager;

    connect(m_DataManager, SIGNAL(SubjectAdded(long)), 
        this, SLOT(SubjectAddedHandler(long))
    );
	connect(m_DataManager, SIGNAL(SubjectRemoved(long)), 
        this, SLOT(SubjectRemovedHandler(long))
    );
	connect(m_DataManager, SIGNAL(SubjectDataChanged(long)), 
        this, SLOT(SubjectDataChangedHandler(long))
    );
}

void ObserverBase::SetDataView(DataViewBase* dataView)
{
    qDebug() << "ObserverBase::SetDataView";

    m_DataView = dataView;

    connect(m_DataView, SIGNAL(SelectedSubjectChanged(long)), 
        this, SLOT(SelectedSubjectChangedHandler(long))
    );
	connect(m_DataView, SIGNAL(SelectedDataChanged(long)), 
        this, SLOT(SelectedDataChangedHandler(long))
    );
	connect(m_DataView, SIGNAL(DataAddedForSelectedSubject(long)), 
        this, SLOT(DataAddedForSelectedSubjectHandler(long))
    );
	connect(m_DataView, SIGNAL(DataRemovedFromSelectedSubject(long)), 
        this, SLOT(DataRemovedFromSelectedSubjectHandler(long))
    );
	connect(m_DataView, SIGNAL(DataCheckedStateChanged(long, bool)), 
        this, SLOT(DataCheckedStateChangedHandler(long, bool))
    );
    qDebug() << "ObserverBase::SetDataView ok";
}

QString ObserverBase::GetName()
{
    return m_Name;
}

void ObserverBase::SelectedSubjectChangedHandler(long uid)
{

}

void ObserverBase::SelectedDataChangedHandler(long iid)
{

}

void ObserverBase::DataAddedForSelectedSubjectHandler(long iid)
{

}

void ObserverBase::DataRemovedFromSelectedSubjectHandler(long iid)
{

}

void ObserverBase::DataCheckedStateChangedHandler(long iid, bool checkState)
{

}

void ObserverBase::SubjectAddedHandler(long uid)
{

}

void ObserverBase::SubjectRemovedHandler(long uid)
{

}

void ObserverBase::SubjectDataChangedHandler(long uid)
{

}

void ObserverBase::SetName(QString name)
{
    m_Name = name;
}