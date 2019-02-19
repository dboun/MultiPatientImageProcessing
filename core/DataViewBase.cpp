#include <QDebug>

#include "DataViewBase.h"

DataViewBase::DataViewBase(QWidget *parent) : QWidget(parent)
{
	qDebug() << QString("DataViewBase::DataViewBase()");
}

void DataViewBase::SetDataManager(DataManager* dataManager)
{
	qDebug() << QString("DataViewBase::SetDataManager()");
	m_DataManager = dataManager;

	connect(dataManager, SIGNAL(SubjectAdded(long)),
            this, SLOT(SubjectAddedHandler(long))
	);

	connect(dataManager, SIGNAL(SubjectRemoved(long)),
            this, SLOT(SubjectRemovedHandler(long))
	);


	connect(dataManager, SIGNAL(SubjectDataChanged(long)),
            this, SLOT(SubjectDataChangedHandler(long))
	);

	m_AcceptedFileTypes = dataManager->GetAcceptedFileTypes();
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

void DataViewBase::UpdateProgressHandler(long uid, int progress)
{
	qDebug() << QString("DataViewBase::UpdateProgressHandler(int, long)");	
}