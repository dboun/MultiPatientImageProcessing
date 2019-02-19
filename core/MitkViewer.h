#ifndef MITK_VIEWER_H
#define MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <mitkStandaloneDataStorage.h>
#include <QmitkStdMultiWidget.h>

#include "ViewerBase.h"

class MitkViewer : public ImageViewerBase
{
	Q_OBJECT

public:
	MitkViewer(QWidget *parent = nullptr);

public slots:
	// Slot for slider
	void SliderHandler(long sliderNumber, int value);

	// Slots for DataViewBase
	void SelectedSubjectChangedHandler(long uid);
	void SelectedDataChangedHandler(long iid);
	void DataCheckedStateChangedHandler(long iid, bool checkState);

	// Slot, but probably just called.
	void SaveImageToFile(long iid);

private:
	QmitkStdMultiWidget* m_MitkWidget;
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
	long currentSubjectId = -1, currentImageId = -1;
};

#endif // ! MITK_VIEWER_H