#ifndef GEODESIC_TRAINING_GUI_H
#define GEODESIC_TRAINING_GUI_H

#include <QString>
#include <QProgressDialog>
#include <QFutureWatcher>

#include <set>

#include "GuiModuleBase.h"
#include "AlgorithmModuleBase.h"

class DataViewBase;
class SchedulerBase;
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

	void SetScheduler(SchedulerBase* scheduler);

	void SetEnabled(bool enabled) override;

public slots:
	// Slots for DataViewBase
	void SelectedSubjectChangedHandler(long uid);
	void DataAddedForSelectedSubjectHandler(long iid);
	void DataRemovedFromSelectedSubjectHandler(long iid);

	// Internal slots
	void OnRunClicked();

	// Slots for Scheduler
	void OnSchedulerJobFinished(AlgorithmModuleBase* algorithmModuleBase);

	// Slots for AlgorithmModuleBase
	void OnAlgorithmFinished(AlgorithmModuleBase* algorithmModuleBase);
	void OnAlgorithmFinishedWithError(AlgorithmModuleBase* algorithmModuleBase, 
		QString errorMessage
	);

signals:
	void ChangeFocusSeeds(long iid);

private:
	DataViewBase*          m_DataView;
	CustomMitkDataStorage* m_DataStorage;
	MitkSegmentationTool*  m_MitkSegmentationTool;
	SchedulerBase*         m_Scheduler;

	std::set<long> m_SubjectsThatAreRunning;

	Ui::GeodesicTrainingGUI *ui;
};

#endif // ! GEODESIC_TRAINING_GUI_H
