#include "GeodesicTrainingWarningGUI.h"

#include <QVBoxLayout>

#include "InfoLabel.h"
#include "WarningInformation.h"
#include "WarningCritical.h"

GeodesicTrainingWarningGUI::GeodesicTrainingWarningGUI(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);
}

GeodesicTrainingWarningGUI::~GeodesicTrainingWarningGUI()
{

}

void GeodesicTrainingWarningGUI::SetWidgetContainer(QWidget* container)
{
    m_Container = container;
    m_Container->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
	QGridLayout *layout = new QGridLayout(m_Container);
	layout->addWidget(this, 0, 0);
    layout->setContentsMargins(0,0,0,0);
	m_Container->setLayout(layout);
}

void GeodesicTrainingWarningGUI::OnNewErrorMessage(QString errorMessage)
{
    m_Container->show();

    if (m_Widgets.find(errorMessage) == m_Widgets.end())
    {
        HandleMessage(errorMessage);
    }
}

void GeodesicTrainingWarningGUI::OnErrorMessageWasRemoved(QString errorMessage)
{
    std::map<QString, QWidget*>::iterator wIter = m_Widgets.find(errorMessage);

    if (wIter != m_Widgets.end())
    {
        QWidget* widget = m_Widgets[errorMessage];
        
        m_Widgets.erase( wIter );

        this->layout()->removeWidget(widget);
        delete widget;
    }

    if (m_Widgets.size() == 0)
    {
        m_Container->hide();
    }
}


void GeodesicTrainingWarningGUI::OnNewWarning(QString warning)
{
    m_Container->show();

    if (m_Widgets.find(warning) == m_Widgets.end())
    {
        HandleMessage(warning);
    }
}

void GeodesicTrainingWarningGUI::OnWarningWasRemoved(QString warningThatWasRemoved)
{
    std::map<QString, QWidget*>::iterator wIter = m_Widgets.find(warningThatWasRemoved);

    if (wIter != m_Widgets.end())
    {
        QWidget* widget = m_Widgets[warningThatWasRemoved];
        
        m_Widgets.erase( wIter );

        this->layout()->removeWidget(widget);
        delete widget;
    }

    if (m_Widgets.size() == 0)
    {
        m_Container->hide();
    }
}

void GeodesicTrainingWarningGUI::HandleMessage(QString message)
{
    QWidget* widget;

    if (message.startsWith("No subjects loaded"))
    {
        WarningInformation* w = new WarningInformation(this);
        w->ShowText(true, message);
        w->ShowButton(false);
        widget = w;
    }
    else if (message == "No patient images")
    {
        WarningInformation* w = new WarningInformation(this);
        w->ShowText(true, message);
        w->ShowButton(false);
        widget = w;
    }
    else {
        // Default case
        WarningCritical* w = new WarningCritical(this);
        w->ShowText(true, message);
        w->ShowButton(false);
        widget = w;
    }

    /*"<b>Critical: </b>" + */
    this->layout()->addWidget(widget);
    m_Widgets[message] = widget;
}