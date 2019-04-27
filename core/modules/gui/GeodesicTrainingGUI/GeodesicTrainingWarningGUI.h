#ifndef GEODESIC_TRAINING_WARNING_GUI
#define GEODESIC_TRAINING_WARNING_GUI

#include <QWidget>

#include <map>

#include "WarningFunctionBase.h"

class GeodesicTrainingWarningGUI : public QWidget
{
    Q_OBJECT

public:
    GeodesicTrainingWarningGUI(QWidget* parent = nullptr);
    ~GeodesicTrainingWarningGUI();

    void SetWidgetContainer(QWidget* container);

public slots:
    void OnNewErrorMessage(QString errorMessage);
    void OnErrorMessageWasRemoved(QString errorMessage);

    void OnNewWarning(QString warning);
    void OnWarningWasRemoved(QString warningThatWasRemoved);

    void OnNewWarningFunctionAdded(WarningFunctionBase* function);
    void OnImageSizeWarningFunctionAllowanceChanged(WarningFunctionBase* function, bool allow);

signals:
    void CreatingSeedsAllowance(bool allow);

private:
    void HandleMessage(QString message);

    QWidget* m_Container;
    std::map< QString, QWidget* > m_Widgets;
};

#endif // ! GEODESIC_TRAINING_WARNING_GUI