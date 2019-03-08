#ifndef IMAGE_VIEWER_BASE_H
#define IMAGE_VIEWER_BASE_H

#include <QWidget>
#include <QSlider>

#include <vector>

#include "DataManager.h"
#include "DataViewBase.h"

class ImageViewerBase : public QWidget
{
	Q_OBJECT

public:
	ImageViewerBase(QWidget *parent = nullptr);

	void SetDataView(DataViewBase* dataView);
	void SetDataManager(DataManager* dataManager);
	void SetOpacitySlider(QSlider* slider);

public slots:
	// Slot for slider
	virtual void OpacitySliderHandler(int value);

	// Slots for DataViewBase
	virtual void SelectedSubjectChangedHandler(long uid);
	virtual void SelectedDataChangedHandler(long iid);
	virtual void DataAddedForSelectedSubjectHandler(long iid);
	virtual void DataRemovedFromSelectedSubjectHandler(long iid);
	virtual void DataCheckedStateChangedHandler(long iid, bool checkState); //optional

	// Slot, but probably just called.
	virtual void SaveImageToFile(long iid);

protected:
	DataManager*  m_DataManager;
	DataViewBase* m_DataView;
};

#endif // ! IMAGE_VIEWER_BASE_H