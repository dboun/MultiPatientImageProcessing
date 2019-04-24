#ifndef GUI_MODULE_BASE_H
#define GUI_MODULE_BASE_H

#include <QWidget>
#include <QString>

#include "DataManager.h"

class GuiModuleBase : public QWidget
{
  Q_OBJECT

public:
  explicit GuiModuleBase(QWidget *parent = nullptr);
  ~GuiModuleBase() {}

  /** These should be set by the main window or whatever uses the widget */
  virtual void SetDataManager(DataManager* dataManager);
  virtual void SetEnabled(bool enabled); 
  void SetAppName(QString appName);
  void SetAppNameShort(QString appNameShort);
  
  static void PlaceWidgetInWidget(QWidget* top, QWidget* bottom);

protected:
  DataManager* GetDataManager();
  

  DataManager* m_DataManager;
  QString      m_AppName = "Multi-Patient Image Processing";
  QString      m_AppNameShort = "MPIP";
};

#endif // ! GUI_MODULE_BASE_H