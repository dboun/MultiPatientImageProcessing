#include "GeodesicTrainingWarningGUI.h"

#include <QVBoxLayout>

#include "InfoLabel.h"
#include "WarningInformation.h"
#include "WarningImportant.h"
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
        // InfoLabel* widget = new InfoLabel(this);
        // widget->setWordWrap(true);
        // widget->setText(errorMessage);
        // this->layout()->addWidget(widget);
        // m_Widgets[errorMessage] = widget;
        WarningImportant* w = new WarningImportant(this);
        w->ShowText(true, "<b>Critical: </b>" + errorMessage);
        //w->ShowButton(true, "OK");
        w->ShowButton(false);
        this->layout()->addWidget(w);
        m_Widgets[errorMessage] = w;
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
        // InfoLabel* widget = new InfoLabel(this);
        // widget->setWordWrap(true);
        // widget->setText(warning);
        // this->layout()->addWidget(widget);
        // m_Widgets[warning] = widget;

        WarningCritical* w = new WarningCritical(this);
        w->ShowText(true, warning);
        //w->ShowButton(true, "OK");
        w->ShowButton(false);
        this->layout()->addWidget(w);
        m_Widgets[warning] = w;
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
