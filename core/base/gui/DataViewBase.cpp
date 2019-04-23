#include <QDebug>

#include "DataViewBase.h"

DataViewBase::DataViewBase(QWidget *parent) : GuiModuleBase(parent)
{
	qDebug() << QString("DataViewBase::DataViewBase()");
}

void DataViewBase::SetDataManager(DataManager* dataManager)
{
	qDebug() << QString("DataViewBase::SetDataManager()");
	
	m_DataManager = dataManager;
	m_AcceptedFileTypes = dataManager->GetAcceptedFileTypes();

	connect(dataManager, SIGNAL(SubjectAdded(long)),
            this, SLOT(SubjectAddedHandler(long))
	);

	connect(dataManager, SIGNAL(SubjectRemoved(long)),
            this, SLOT(SubjectRemovedHandler(long))
	);


	connect(dataManager, SIGNAL(SubjectDataChanged(long)),
            this, SLOT(SubjectDataChangedHandler(long))
	);
}

long DataViewBase::GetCurrentSubjectID()
{
	return m_CurrentSubjectID;
}

long DataViewBase::GetCurrentDataID()
{
	return m_CurrentDataID;
}

bool DataViewBase::IsDataChecked(long iid)
{
	return true;
}

void DataViewBase::SetDataCheckedState(long iid, bool checkState)
{
	qDebug() << QString("DataViewBase::SetDataCheckedState(bool)");
}

void DataViewBase::SubjectAddedHandler(long uid)
{
	qDebug() << QString("DataViewBase::SubjectAddedHandler(long)");
}

void DataViewBase::SubjectRemovedHandler(long uid)
{
	qDebug() << QString("DataViewBase::SubjectRemovedHandler(long)");
}

void DataViewBase::SubjectDataChangedHandler(long uid)
{
	qDebug() << QString("DataViewBase::SubjectDataChangedHandler(long)");
}

void DataViewBase::UpdateProgressHandler(long uid, QString message, int progress)
{
	qDebug() << QString("DataViewBase::UpdateProgressHandler(int, long)");	
}

void DataViewBase::SetCurrentSubjectID(long uid)
{
	m_CurrentSubjectID = uid;
}

void DataViewBase::SetCurrentDataID(long iid)
{
	m_CurrentDataID = iid;
}