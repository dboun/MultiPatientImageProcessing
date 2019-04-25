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

private:
    Ui::SideWidget *ui;

};

#endif // ! SIDE_WIDGET_H
