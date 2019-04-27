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
        ui->warningInfoFrame->layout()->setSpacing(0);
    }

    // There is a weird space left so this is a workaround.
    // But can't really use the button
    // ui->warningFrame->layout()->removeWidget(ui->warningInfoPushButton);

    return ui->warningInfoPushButton;
}

QLayout* WarningInformation::GetLayout()
{
    return ui->warningInfoFrame->layout();
}