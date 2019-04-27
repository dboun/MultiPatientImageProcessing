#ifndef WARNING_CRITICAL_H
#define WARNING_CRITICAL_H

#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLayout>

namespace Ui {
class WarningCritical;
}

class WarningCritical : public QWidget
{
    Q_OBJECT

public:
    explicit WarningCritical(QWidget* parent = nullptr);

    ~WarningCritical();

    void ShowText(bool showText, QString text = "");
    
    QPushButton* ShowButton(bool showButton, QString buttonText = "");
    
    QLayout* GetLayout();

private:
    Ui::WarningCritical *ui;

};

#endif // ! WARNING_CRITICAL_H