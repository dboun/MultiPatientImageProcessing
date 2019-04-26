#include "WarningInformation.h"

#include "ui_WarningInformation.h"

WarningInformation::WarningInformation(QWidget* parent) : 
    QWidget(parent),
    ui(new Ui::WarningInformation)
{
    ui->setupUi(this);
}

WarningInformation::~WarningInformation()
{
    delete ui;
}

void WarningInformation::ShowText(bool showText, QString text)
{
    if (text == "") { showText = false; }

    if (showText)
    {
        ui->warningInfoLabel->setText(text);
        ui->warningInfoLabel->show();
    }
    else {
        ui->warningInfoLabel->hide();
    }
}

QPushButton* WarningInformation::ShowButton(bool showButton, QString buttonText)
{
    if (buttonText == "") { showButton = false; }

    if (showButton)
    {
        ui->warningInfoPushButton->setText(buttonText);
        ui->warningInfoPushButton->show();
    }
    else {
        ui->warningInfoPushButton->hide();
    }

    return ui->warningInfoPushButton;
}

QLayout* WarningInformation::GetLayout()
{
    return ui->frame->layout();
}