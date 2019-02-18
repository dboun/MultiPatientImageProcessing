#ifndef MPIP_MITK_VIEWER_H
#define MPIP_MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <mitkStandaloneDataStorage.h>
#include <QmitkStdMultiWidget.h>

#include "ViewerBase.h"

class MpipMitkViewer : public QmitkStdMultiWidget, public ViewerBase
{
	Q_OBJECT

public:
	MpipMitkViewer(QObject *parent = nullptr);

	void Display(QString imagePath, QString overlayPath = QString()) override;
	void ChangeOpacity(float value) override;
	bool RemoveImageOrOverlayIfLoaded(QString path) override;
	void SaveOverlayToFile(QString fullPath) override;

private:
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
	QString lastImagePath, lastOverlayPath;
};

#endif // ! MPIP_MITK_VIEWER_H