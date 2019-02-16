#ifndef MPIP_MITK_VIEWER_H
#define MPIP_MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <itkImage.h>
#include <mitkStandaloneDataStorage.h>
#include "QmitkStdMultiWidget.h"

#include "ViewerBase.h"

class MpipMitkViewer : public ViewerBase, public QmitkStdMultiWidget
{
	Q_OBJECT

public:
	MpipMitkViewer();

	// Methods overriden from ViewerBase
	void Display(QString imagePath, QString overlayPath = QString()) override;
	void ChangeOpacity(float value) override;
	bool RemoveImageOrOverlayIfLoaded(QString path) override;
	void SaveOverlayToFile(QString fullPath) override;

private:
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
	QString lastImagePath, lastOverlayPath;
};

#endif // ! MPIP_MITK_VIEWER_H