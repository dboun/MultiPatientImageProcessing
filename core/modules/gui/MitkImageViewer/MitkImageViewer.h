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
	virtual void OnExportData(long iid, QString fileName);

	// Slot, but usually just called.
	void SaveImageToFile(long iid, bool updateDataManager = true);

	// Reference is an nrrd image that shows which labels to use
	void ConvertToNrrdAndSave(long iid, long referenceIid = -1, bool updateDataManager = true);

    //long CreateEmptyMask(long referenceIid);

signals:
	void MitkLoadedNewMask(mitk::DataNode::Pointer dataNode);
	void MitkNodeAboutToBeDeleted(long iid);
protected:
	virtual void AddToDataStorage(long iid);

	QmitkStdMultiWidget* m_MitkWidget;
	mitk::StandaloneDataStorage::Pointer m_DataStorage;

private:
	bool m_FirstTimeForThisSubject = true;
};

#endif // ! MITK_IMAGE_VIEWER_H
