#ifndef GEODESIC_TRAINING_GUI_H
#define GEODESIC_TRAINING_GUI_H

#include <QString>
#include <QProgressDialog>
#include <QFutureWatcher>

#include <mitkStandaloneDataStorage.h>

#include "GuiModuleBase.h"

class MitkDrawingTool;
class MitkImageViewer;
class DataViewBase;

namespace Ui {
class GeodesicTrainingGUI;
}

class GeodesicTrainingGUI : public GuiModuleBase
{
    Q_OBJECT

public:
    explicit GeodesicTrainingGUI(mitk::DataStorage *datastorage, QWidget *parent = nullptr);
    ~GeodesicTrainingGUI();

	void SetMitkImageViewer(MitkImageViewer* mitkImageViewer);

	void SetDataManager(DataManager* dataManager) override;

	void SetDataView(DataViewBase* dataViewBase);

private:

	void OnRunClicked();

	MitkDrawingTool* m_MitkDrawingTool;
	MitkImageViewer* m_MitkImageViewer;
	DataViewBase*    m_DataView;

	Ui::GeodesicTrainingGUI *ui;
	mitk::DataStorage *m_DataStorage;
};

#endif // ! GEODESIC_TRAINING_GUI_H
