#include "ViewerBase.h"
#include <QDebug>

ViewerBase::ViewerBase(QWidget *parent) : QWidget(parent)
{
	qDebug() << QString("ViewerBase::ViewerBase()");
}

void ViewerBase::SetDataView(DataViewBase* dataView)
{
	qDebug() << QString("ViewerBase::SetDataView()");
	// TODO connect signals and slots

	connect(dataView, SIGNAL(DataViewBase::SelectedSubjectChanged(long)),
            this, SLOT(SelectedSubjectChangedHandler(long))
	);

	connect(dataView, SIGNAL(DataViewBase::SelectedDataChanged(long)),
            this, SLOT(SelectedDataChangedHandler(long))
	);
}

void ViewerBase::SetDataManager(DataManager* dataManager)
{
	qDebug() << QString("ViewerBase::SetDataManager()");
	m_DataManager = dataManager;
	// I don't think there is a need to connect signals/slots
	// DataManager can be used for data fetching
}

long ViewerBase::AddSlider(QSlider* slider)
{
	qDebug() << QString("ViewerBase::AddSlider(slider)");
	ConnectSlider(slider);
	return m_SlidersCount++; // id for the slider
}

void ViewerBase::Display(QString imagePath, QString overlayPath)
{
	qDebug() << QString("ViewerBase::Display(") << imagePath 
             << QString(", ") << overlayPath << QString(")");
}

bool ViewerBase::RemoveImageOrOverlayIfLoaded(QString path)
{
	qDebug() << QString("ViewerBase::RemoveImageOrOverlayIfLoaded(")
             << path << QString(")");
	return true;
}

void ViewerBase::SaveOverlayToFile(QString fullPath)
{
	qDebug() << QString("ViewerBase::SaveOverlayToFile(")
             << fullPath << QString(")");
}