#include "GuiModuleBase.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>

GuiModuleBase::GuiModuleBase(QWidget *parent) : QWidget(parent)
{

}

void GuiModuleBase::SetDataManager(DataManager* dataManager)
{
    m_DataManager = dataManager;
}

void GuiModuleBase::SetEnabled(bool enabled)
{

}

void GuiModuleBase::SetAppName(QString appName)
{
    m_AppName = appName;
}

void GuiModuleBase::SetAppNameShort(QString appNameShort)
{
    m_AppNameShort = appNameShort;
}

DataManager* GuiModuleBase::GetDataManager()
{
    return m_DataManager;
}

void GuiModuleBase::PlaceWidgetInWidget(QWidget* top, QWidget* bottom)
{
    top->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QGridLayout *layout = new QGridLayout(bottom);
	layout->addWidget(top, 0, 0);
	bottom->setLayout(layout);
}