#ifndef MITK_SEGMENTATION_TOOL_H
#define MITK_SEGMENTATION_TOOL_H

#include <QString>
#include <QProgressDialog>
#include <QFutureWatcher>

#include <mitkToolManagerProvider.h>
#include <mitkStandaloneDataStorage.h>

#include "GuiModuleBase.h"
#include "CustomMitkDataStorage.h"

class QmitkToolGUI;

namespace Ui {
class MitkSegmentationTool;
}

class MitkSegmentationTool : public GuiModuleBase
{
    Q_OBJECT

public:
    explicit MitkSegmentationTool(QWidget *parent = nullptr);
    ~MitkSegmentationTool();

	void SetDataManager(DataManager* dataManager) override;

	// It could be "Segmentation" or "Mask"
	void SetSpecialRoleOfInterest(QString specialRoleOfInterest);

public slots:
	// This could connect to DataViewBase::SelectedDataChanged(iid)
	// Or manually set it to the mask of the subject
	void ChangeFocusImage(long iid);

	// Slots for DataManager
	void OnDataAboutToGetRemoved(long iid);

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
    void RemoveExistingToolGui();

	bool m_WaitingOnLabelsImageCreation = false;
	bool m_MaskLoadedForThisSubject     = false;
	
	mitk::DataNode::Pointer m_LoadedMaskNode;
	
	Ui::MitkSegmentationTool *ui;
	QProgressDialog          *m_ProgressDialog;
	QFutureWatcher<void>     *m_ProgressDialogWatcher;
	
	mitk::ToolManager::Pointer m_ToolManager;
	CustomMitkDataStorage*     m_DataStorage;
	QmitkToolGUI*              m_LastToolGUI;

	QString m_SpecialRoleOfInterest = "Segmentation";
	long    m_CurrentFocusImage     = -1;
};

#endif // ! MITK_SEGMENTATION_TOOL_H
