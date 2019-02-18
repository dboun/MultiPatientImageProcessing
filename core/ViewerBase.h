#ifndef VIEWER_BASE_H
#define VIEWER_BASE_H

#include <QWidget>
#include <QSlider>

#include <vector>

#include "DataManager.h"
#include "DataViewBase.h"

class ViewerBase : public QWidget
{
	Q_OBJECT

public:
	ViewerBase(QWidget *parent = nullptr);

	void SetDataView(DataViewBase* dataView);
	void SetDataManager(DataManager* dataManager);
	long AddSlider(QSlider* slider);

public slots:
	virtual void Display(QString imagePath, QString overlayPath = QString());
	virtual bool RemoveImageOrOverlayIfLoaded(QString path);
	virtual void SaveOverlayToFile(QString fullPath);

	// Slot for slider
  virtual void SliderHandler(long sliderNumber, int value) {}

	// Slots for DataViewBase
  virtual void SelectedSubjectChangedHandler(long uid) {}
  virtual void SelectedDataChangedHandler(long iid) {}

protected:

	// Override this to connect custom signal 
	// (triggered from slider's valueChanged) signal 
	// to slot SliderHandler
  virtual void ConnectSlider(QSlider* slider) {}

	DataManager* m_DataManager;
	long m_SlidersCount;
};

#endif // ! VIEWER_BASE_H