#include "ViewerBase.h"
#include <QDebug>

ViewerBase::ViewerBase()
{
	qDebug() << QString("ViewerBase::ViewerBase()");
}

void ViewerBase::Display(QString imagePath, QString overlayPath)
{
	qDebug() << QString("ViewerBase::Display(") << imagePath 
             << QString(", ") << overlayPath << QString(")");
}

void ViewerBase::ChangeOpacity(float value)
{
	qDebug() << QString("ViewerBase::ChangeOpacity(") 
             << value << QString(")");
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