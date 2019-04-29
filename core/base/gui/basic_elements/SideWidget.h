#ifndef SIDE_WIDGET_H
#define SIDE_WIDGET_H

#include <QWidget>

namespace Ui {
class SideWidget;
}

class SideWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SideWidget(QWidget* parent = nullptr);

    ~SideWidget();

    void AddCustomWidget(QWidget* widget);
    QWidget* GetCustomWidget();

private:
    Ui::SideWidget *ui;
    QWidget* m_CustomWidget;
};

#endif // ! SIDE_WIDGET_H
