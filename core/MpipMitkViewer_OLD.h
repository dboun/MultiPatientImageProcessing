#ifndef MPIP_MITK_VIEWER_H
#define MPIP_MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <mitkStandaloneDataStorage.h>
#include <QmitkStdMultiWidget.h>

#include "ViewerBase.h"

class MpipMitkViewer : public ViewerBase
{
	Q_OBJECT

public:
	MpipMitkViewer(QWidget *parent = nullptr);

	void Display(QString imagePath, QString overlayPath = QString()) override;
//	void ChangeOpacity(float value) override;
	bool RemoveImageOrOverlayIfLoaded(QString path) override;
	void SaveOverlayToFile(QString fullPath) override;

private:
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
  QmitkStdMultiWidget* m_MitkWidget;
	QString lastImagePath, lastOverlayPath;
};

#endif // ! MPIP_MITK_VIEWER_H