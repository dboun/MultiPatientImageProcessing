#ifndef MPIPQMITKSEGMENTATIONPANEL_H
#define MPIPQMITKSEGMENTATIONPANEL_H

#include <QWidget>
#include <mitkToolManagerProvider.h>
#include <mitkStandaloneDataStorage.h>

class QmitkToolGUI;
namespace Ui {
class MPIPQmitkSegmentationPanel;
}

class MPIPQmitkSegmentationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MPIPQmitkSegmentationPanel(mitk::DataStorage *datastorage, QWidget *parent = nullptr);
    ~MPIPQmitkSegmentationPanel();

public slots:
  void OnCreateNewLabel();
  void OnEnableSegmentation();
  void OnNewSegmentationSession();
  void OnConfirmSegmentation();
  void SetDisplayDataName(QString);
  void OnManualTool2DSelected(int);

private:
  void CreateNewSegmentation();
  void RemoveExistingToolGui();

    Ui::MPIPQmitkSegmentationPanel *ui;
    mitk::ToolManager::Pointer toolManager;
    mitk::DataStorage *m_DataStorage;
    QString segName;
    QString m_DisplayDataName;
    QmitkToolGUI *m_LastToolGUI;
};

#endif // MPIPQMITKSEGMENTATIONPANEL_H
