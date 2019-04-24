#ifndef GEODESIC_TRAINING_GUI_H
#define GEODESIC_TRAINING_GUI_H

#include <QString>
#include <QProgressDialog>
#include <QFutureWatcher>

#include "GuiModuleBase.h"

class DataViewBase;
class MitkSegmentationTool;
class CustomMitkDataStorage;

namespace Ui {
class GeodesicTrainingGUI;
}

class GeodesicTrainingGUI : public GuiModuleBase
{
    Q_OBJECT

public:
    explicit GeodesicTrainingGUI(QWidget *parent = nullptr);
    ~GeodesicTrainingGUI();

	void SetDataManager(DataManager* dataManager) override;

	void SetDataView(DataViewBase* dataView);

	void SetEnabled(bool enabled) override;

public slots:
	// Slots for DataViewBase
	void SelectedSubjectChangedHandler(long uid);
	void DataAddedForSelectedSubjectHandler(long iid);
	void DataRemovedFromSelectedSubjectHandler(long iid);

	// Internal slots
	void OnRunClicked();

signals:
	void ChangeFocusSeeds(long iid);

private:
	DataViewBase*          m_DataView;
	CustomMitkDataStorage* m_DataStorage;
	MitkSegmentationTool*  m_MitkSegmentationTool;

	Ui::GeodesicTrainingGUI *ui;
};

#endif // ! GEODESIC_TRAINING_GUI_H
