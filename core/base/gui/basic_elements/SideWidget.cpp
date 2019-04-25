#include "SideWidget.h"

#include "ui_SideWidget.h"

SideWidget::SideWidget(QWidget* parent) : QWidget(parent), ui(new Ui::SideWidget) {
    ui->setupUi(this);
}

SideWidget::~SideWidget() {
    delete ui;    
}

void SideWidget::AddCustomWidget(QWidget* widget) {
    ui->contentArea->layout()->addWidget(widget);
}