#ifndef WARNING_INFORMATION_H
#define WARNING_INFORMATION_H

#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLayout>

namespace Ui {
class WarningInformation;
}

class WarningInformation : public QWidget
{
    Q_OBJECT

public:
    explicit WarningInformation(QWidget* parent = nullptr);

    ~WarningInformation();

    void ShowText(bool showText, QString text = "");
    
    QPushButton* ShowButton(bool showButton, QString buttonText = "");
    
    QLayout* GetLayout();

private:
    Ui::WarningInformation *ui;

};

#endif // ! WARNING_INFORMATION_H