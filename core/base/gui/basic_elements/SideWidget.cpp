#include "SideWidget.h"

#include "ui_SideWidget.h"

SideWidget::SideWidget(QWidget* parent) : QWidget(parent), ui(new Ui::SideWidget) {
    ui->setupUi(this);
	//ui->gridLayout->setContentsMargins(0, 0, 0, 0);
	//ui->gridLayout_2->setContentsMargins(0, 0, 0, 0);
	//ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
	//ui->scrollArea->setContentsMargins(0, 0, 0, 0);
	//ui->scrollAreaWidgetContents->setContentsMargins(0, 0, 0, 0);
	//ui->contentArea->layout()->setContentsMargins(0, 0, 0, 0);
}

SideWidget::~SideWidget() {
    delete ui;    
}

void SideWidget::AddCustomWidget(QWidget* widget) {
    ui->contentArea->layout()->addWidget(widget);
	m_CustomWidget = widget;
}

QWidget* SideWidget::GetCustomWidget()
{
	return m_CustomWidget;
}