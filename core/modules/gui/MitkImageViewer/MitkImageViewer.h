#ifndef MITK_IMAGE_VIEWER_H
#define MITK_IMAGE_VIEWER_H

#include <QString>
#include <QWidget>
#include <QList>
#include <mitkDataNode.h>
#include <mitkStandaloneDataStorage.h>
#include <QmitkStdMultiWidget.h>

#include "ImageViewerBase.h"

class MitkImageViewer : public ImageViewerBase
{
	Q_OBJECT

public:
	MitkImageViewer(QWidget *parent = nullptr);
	~MitkImageViewer();
	
    inline mitk::DataStorage* GetDataStorage()
    {
        return m_DataStorage;
    }

public slots:
	// Slot for slider
	virtual void OpacitySliderHandler(int value) override;

	// Slots for DataViewBase
	virtual void SelectedSubjectChangedHandler(long uid) override;
	virtual void DataAddedForSelectedSubjectHandler(long iid) override;
	virtual void DataRemovedFromSelectedSubjectHandler(long iid) override;
	virtual void SelectedDataChangedHandler(long iid) override;
	virtual void DataCheckedStateChangedHandler(long iid, bool checkState) override;

	// Slot, but probably just called.
	void SaveImageToFile(long iid) override;

signals:
	void LoadedNewMask(mitk::DataNode::Pointer dataNode);
	void MitkDataNodeAboutToGetRemoved(mitk::DataNode::Pointer dataNode);

protected:
	virtual void AddToDataStorage(long iid);

	QmitkStdMultiWidget* m_MitkWidget;
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
};

#endif // ! MITK_IMAGE_VIEWER_H
