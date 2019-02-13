#ifndef MPIP_MITK_VIEWER_H
#define MPIP_MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <QDebug>
#include <mitkStandaloneDataStorage.h>
//#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include "QmitkStdMultiWidget.h"

class MpipMitkViewer : public QmitkStdMultiWidget /*public QWidget*/
{
	Q_OBJECT

public:
	MpipMitkViewer();

	void Display(QString imagePath);
	void SetupWidgets();

private:
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
};

#endif // ! MPIP_MITK_VIEWER_H