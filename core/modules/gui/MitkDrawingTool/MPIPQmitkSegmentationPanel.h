#ifndef MPIPQMITKSEGMENTATIONPANEL_H
#define MPIPQMITKSEGMENTATIONPANEL_H

#include <QWidget>
#include <mitkToolManagerProvider.h>
#include <mitkStandaloneDataStorage.h>

#include "DataViewBase.h"
#include "DataManager.h"

class QmitkToolGUI;
class DataManager;
namespace Ui {
class MPIPQmitkSegmentationPanel;
}

class MPIPQmitkSegmentationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MPIPQmitkSegmentationPanel(mitk::DataStorage *datastorage, QWidget *parent = nullptr);
    ~MPIPQmitkSegmentationPanel();

    void SetDataManager(DataManager *dataManager);
    void SetDataView(DataViewBase* dataView);
    void SetAppName(QString appName);

public slots:
	void OnNewSegmentationSession();
	void OnResumeSegmentationSession();
	void OnConfirmSegmentation();
	void OnCreateNewLabel();
	void SetDisplayDataName(long);
	//void OnManualTool2DSelected(int);

private:
	//void CreateNewSegmentation();

	void InitializeReferenceNodeAndImageForSubject(long uid);

	Ui::MPIPQmitkSegmentationPanel *ui;
	long m_CurrentSubject = -1;
	long m_CurrentData = -1;
	QString segName;
	QString m_DisplayDataName;
	DataManager*  m_DataManager;
	DataViewBase* m_DataView;
	QString m_AppName = QString("MPIP");

	// MITK Things
	mitk::ToolManager::Pointer     toolManager;
	mitk::DataStorage*             m_DataStorage;
	QmitkToolGUI*                  m_LastToolGUI;

	mitk::Image::Pointer           m_ReferenceImage;
	mitk::DataNode::Pointer        m_ReferenceNode; 
	mitk::Image::Pointer           m_WorkingImage;
	mitk::DataNode::Pointer        m_WorkingNode; 
};

#endif // ! MPIPQMITKSEGMENTATIONPANEL_H
