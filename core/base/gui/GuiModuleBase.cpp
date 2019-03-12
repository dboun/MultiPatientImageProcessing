#include "GuiModuleBase.h"

#include <QDebug>
#include <QGridLayout>

void GuiModuleBase::SetDataManager(DataManager* dataManager)
{
    m_DataManager = dataManager;
}

void GuiModuleBase::SetAppName(QString appName)
{
    m_AppName = appName;
}

void GuiModuleBase::SetAppNameShort(QString appNameShort)
{
    m_AppNameShort = appNameShort;
}

void GuiModuleBase::SetupUI()
{
    qDebug() << "GuiModuleBase::SetupUI()";
}

DataManager* GuiModuleBase::GetDataManager()
{
    return m_DataManager;
}

static void PlaceWidgetInWidget(QWidget* top, QWidget* bottom)
{
	QGridLayout *layout = new QGridLayout(bottom);
	layout->addWidget(top, 0, 0);
	bottom->setLayout(layout);
}