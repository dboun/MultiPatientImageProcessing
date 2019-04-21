#ifndef MITK_SEGMENTATION_TOOL_H
#define MITK_SEGMENTATION_TOOL_H

#include <QString>
#include <QProgressDialog>
#include <QFutureWatcher>

#include <mitkToolManagerProvider.h>
#include <mitkStandaloneDataStorage.h>

#include "GuiModuleBase.h"
#include "MitkImageViewer.h"

class QmitkToolGUI;

namespace Ui {
class MitkSegmentationTool;
}

class MitkSegmentationTool : public GuiModuleBase
{
    Q_OBJECT

public:
    explicit MitkSegmentationTool(mitk::DataStorage *datastorage, QWidget *parent = nullptr);
    ~MitkSegmentationTool();

	void SetMitkImageViewer(MitkImageViewer* mitkImageViewer);

	void SetDataManager(DataManager* dataManager) override;

	void SetDataView(DataViewBase* dataViewBase);

public slots:
	// Slots for DataManager
	void OnDataAboutToGetRemoved(long iid);
	
	// Slots for MitkViewer
	void OnMitkLoadedNewMask(mitk::DataNode::Pointer dataNode);

	// Internal slots
	void OnCreateNewLabel();
	void OnCreateNewMask();
	void OnManualTool2DSelected(int);
    void CreateEmptyMask(long referenceIid);
	void OnCreateEmptyMaskBackgroundFinished();
	void SetMaskFromNiftiData(long);
	void SetSegmentationFromNiftiData(long);

signals:
	void MitkSegmentationToolSaveImageToFile(long iid, bool updateDataManager);
	//void MitkSegmentationToolCreateEmptyMask(long referenceIid);

private:
	// void CreateNewSegmentation();

    void RemoveExistingToolGui();

	bool m_WaitingOnLabelsImageCreation = false;
	bool m_MaskLoadedForThisSubject     = false;
	
	mitk::DataNode::Pointer m_LoadedMaskNode;
	
	Ui::MitkSegmentationTool *ui;
	QProgressDialog      *m_ProgressDialog;
	QFutureWatcher<void> *m_ProgressDialogWatcher;
	
	mitk::ToolManager::Pointer m_ToolManager;
	mitk::DataStorage *m_DataStorage;
	QmitkToolGUI *m_LastToolGUI;
};

#endif // ! MITK_SEGMENTATION_TOOL_H
