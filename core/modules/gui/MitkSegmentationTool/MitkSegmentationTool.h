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

class MitkSegmentationToolController;

class MitkSegmentationTool : public GuiModuleBase
{
    Q_OBJECT

public:
    explicit MitkSegmentationTool(QWidget *parent = nullptr);
    ~MitkSegmentationTool();

	/** If this is set to true multiple segmentations (of this special role) 
	 *  can exist for this subject. */
	void SetAllowMultiple(bool allowMultiple);

	/** Clears the current segmentation and reverts to the state as if 
	 *  no segmentation is loaded. Doesn't actually delete the segmentation. */
	void RevertToNullState();

	/** It could be "Segmentation" or "Mask" or "Seeds" or whatever */
	void SetSpecialRoleOfInterest(QString specialRoleOfInterest);

public slots:
	/** This could connect to DataViewBase::SelectedDataChanged(iid) 
	 *  or to manually use it to set the mask of the subject */
	void ChangeFocusImage(long iid);

	// Internal slots
	void OnManualTool2DSelected(int);
	void OnCreateNewLabelSetImageClicked();
	void OnAddNewLabelClicked();

signals:
	void CreateNewLabelSetImageClicked();
	void AddNewLabelClicked();

private:
    void RemoveExistingToolGui();

	bool m_WaitingOnLabelsImageCreation = false;
	bool m_MaskLoadedForThisSubject     = false;
	
	mitk::DataNode::Pointer m_LoadedMaskNode;

	mitk::DataNode::Pointer m_EmptyImageNode; // Used to reset mitk widgets
	
	Ui::MitkSegmentationTool *ui;
	QProgressDialog          *m_ProgressDialog;
	QFutureWatcher<void>     *m_ProgressDialogWatcher;
	
	mitk::ToolManager::Pointer m_ToolManager;
	CustomMitkDataStorage*     m_DataStorage;
	QmitkToolGUI*              m_LastToolGUI;

	QString m_SpecialRoleOfInterest = "Segmentation";
	long    m_CurrentFocusImageID     = -1;

	bool    m_AllowMultiple = true;
};

#endif // ! MITK_SEGMENTATION_TOOL_H
