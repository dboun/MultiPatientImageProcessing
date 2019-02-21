#ifndef MPIPQMITKSEGMENTATIONPANEL_H
#define MPIPQMITKSEGMENTATIONPANEL_H

#include <QWidget>
#include <mitkToolManagerProvider.h>
#include <mitkStandaloneDataStorage.h>

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

    void SetDataManager(DataManager *manager);

public slots:
  void OnCreateNewLabel();
  void OnEnableSegmentation();
  void OnNewSegmentationSession();
  void OnConfirmSegmentation();
  void SetDisplayDataName(long);
  void OnManualTool2DSelected(int);

private:
  void CreateNewSegmentation();
  void RemoveExistingToolGui();

    Ui::MPIPQmitkSegmentationPanel *ui;
    mitk::ToolManager::Pointer toolManager;
    mitk::DataStorage *m_DataStorage;
    long m_CurrentSubject = -1;
    long m_CurrentData = -1;
    QString segName;
    QString m_DisplayDataName;
    QmitkToolGUI *m_LastToolGUI;
    DataManager *m_datamanager;
};

#endif // MPIPQMITKSEGMENTATIONPANEL_H
