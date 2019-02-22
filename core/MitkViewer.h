#ifndef MITK_VIEWER_H
#define MITK_VIEWER_H

#include <QString>
#include <QWidget>
#include <QList>
#include <mitkStandaloneDataStorage.h>
#include <QmitkStdMultiWidget.h>

#include "ImageViewerBase.h"

class MitkViewer : public ImageViewerBase
{
	Q_OBJECT

public:
	MitkViewer(QWidget *parent = nullptr);
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
    void DisplayedDataName(long);

private:

	//void ConnectSlider(QSlider* slider) override;

	QmitkStdMultiWidget* m_MitkWidget;
	//mitk::DataStorage::Pointer m_DataStorage;
	mitk::StandaloneDataStorage::Pointer m_DataStorage;
	long m_CurrentData = -1;

    QList<long> m_LoadedImages;
};

#endif // ! MITK_VIEWER_H
