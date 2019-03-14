#ifndef MITK_IMAGE_VIEWER_H
#define MITK_IMAGE_VIEWER_H

#include <QString>
#include <QWidget>
#include <QList>
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
	void OpacitySliderHandler(int value) override;

	// Slots for DataViewBase
	void SelectedSubjectChangedHandler(long uid) override;
	void DataAddedForSelectedSubjectHandler(long iid) override;
	void DataRemovedFromSelectedSubjectHandler(long iid) override;
	void SelectedDataChangedHandler(long iid) override;
	void DataCheckedStateChangedHandler(long iid, bool checkState) override;

	// Slot, but probably just called.
	void SaveImageToFile(long iid) override;

signals:
	void MitkDataStorageAboutToGetCleared(mitk::StandaloneDataStorage::Pointer dataStorage);

private:
	QmitkStdMultiWidget* m_MitkWidget;
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
    QList<long> m_LoadedImages;
    long m_CurrentData = -1; // TODO: Delete this
};

#endif // ! MITK_IMAGE_VIEWER_H
