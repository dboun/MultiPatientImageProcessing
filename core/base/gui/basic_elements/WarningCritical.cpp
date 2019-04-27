#include "WarningCritical.h"

#include "ui_WarningCritical.h"

WarningCritical::WarningCritical(QWidget* parent) : 
    QWidget(parent),
    ui(new Ui::WarningCritical)
{
    ui->setupUi(this);
}

WarningCritical::~WarningCritical()
{
    delete ui;
}

void WarningCritical::ShowText(bool showText, QString text)
{
    if (text == "") { showText = false; }

    if (showText)
    {
        ui->warningCriticalLabel->setText(text);
        ui->warningCriticalLabel->show();
    }
    else {
        ui->warningCriticalLabel->hide();
    }
}

QPushButton* WarningCritical::ShowButton(bool showButton, QString buttonText)
{
    if (buttonText == "") { showButton = false; }

    if (showButton)
    {
        ui->warningCriticalPushButton->setText(buttonText);
        ui->warningCriticalPushButton->show();
    }
    else {
        ui->warningCriticalPushButton->hide();
        ui->warningCriticalFrame->layout()->setSpacing(0);
    }

    // There is a weird space left so this is a workaround.
    // But can't really use the button
    // ui->warningFrame->layout()->removeWidget(ui->warningCriticalPushButton);

    return ui->warningCriticalPushButton;
}

QLayout* WarningCritical::GetLayout()
{
    return ui->warningCriticalFrame->layout();
}