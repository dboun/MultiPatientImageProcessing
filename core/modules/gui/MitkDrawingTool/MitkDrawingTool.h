#ifndef MITK_DRAWING_TOOL_H
#define MITK_DRAWING_TOOL_H

#include <mitkToolManagerProvider.h>
#include <mitkStandaloneDataStorage.h>

#include "GuiModuleBase.h"
#include "MitkImageViewer.h"

class QmitkToolGUI;

namespace Ui {
class MitkDrawingTool;
}

class MitkDrawingTool : public GuiModuleBase
{
    Q_OBJECT

public:
    explicit MitkDrawingTool(mitk::DataStorage *datastorage, QWidget *parent = nullptr);
    ~MitkDrawingTool();

	void SetMitkImageViewer(MitkImageViewer* mitkImageViewer);

public slots:
	void OnMitkLoadedNewMask(mitk::DataNode::Pointer dataNode);
	void OnMitkDataNodeAboutToGetRemoved(mitk::DataNode::Pointer dataNode);

	void OnCreateNewLabel();
	void OnConfirmSegmentation();
	void OnManualTool2DSelected(int);
	// void OnDisableSegmentation();

signals:
	void MitkDrawingToolSaveImageToFile(long iid);

private:
	// void CreateNewSegmentation();

	bool m_WaitingOnLabelsImageCreation = false;
	bool m_MaskLoadedForThisSubject     = false;
	
	mitk::DataNode::Pointer m_LoadedMaskNode;

	Ui::MitkDrawingTool *ui;
	
	mitk::ToolManager::Pointer m_ToolManager;
	mitk::DataStorage *m_DataStorage;
	QmitkToolGUI *m_LastToolGUI;
};

#endif // ! MITK_DRAWING_TOOL_H
