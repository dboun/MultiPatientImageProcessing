#ifndef MPIP_MITK_VIEWER_H
#define MPIP_MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <QDebug>
#include <QMouseEvent>
#include <mitkStandaloneDataStorage.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include "QmitkStdMultiWidget.h"

class MpipMitkViewer : public QmitkStdMultiWidget
{
	Q_OBJECT

public:
	MpipMitkViewer();

	void Display(QString imagePath, QString overlayPath = QString());
	void SetupWidgets();
	void ChangeOpacity(float value);

private:
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
	QString lastImagePath, lastOverlayPath;
};

#endif // ! MPIP_MITK_VIEWER_H