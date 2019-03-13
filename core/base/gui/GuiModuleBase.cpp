#include "GuiModuleBase.h"

#include <QDebug>
#include <QGridLayout>

GuiModuleBase::GuiModuleBase(QWidget *parent) : QWidget(parent)
{

}

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

void GuiModuleBase::PlaceWidgetInWidget(QWidget* top, QWidget* bottom)
{
	QGridLayout *layout = new QGridLayout(bottom);
	layout->addWidget(top, 0, 0);
	bottom->setLayout(layout);
}