#ifndef WARNING_MANAGER_H
#define WARNING_MANAGER_H

#include <QObject>

#include <vector>

#include "WarningFunctionBase.h"

class WarningManager : public QObject
{
    Q_OBJECT

public:
    WarningManager(QObject* parent = nullptr);

    ~WarningManager();

    /** The WarningManager doesn't check if any have become nullptr.
     *  GetFunctions might not be useful anyway. */
    std::vector< WarningFunctionBase* > GetFunctions();

    /** Returns true if the operation is allowed by all registered functions */
    bool IsOperationAllowed();

    void RegisterWarningFunction(WarningFunctionBase* function);
    void UnregisterWarningFunction(WarningFunctionBase* function);
    void UnregisterAllWarningFunctionsWithName(QString functionName);

public slots:
    /** Override these functions to potentially add more functionality */
    virtual void OnNewWarningFunctionAdded(WarningFunctionBase* function);
    virtual void OnWarningFunctionAboutToBeRemoved(WarningFunctionBase* function);

    // Slots for WarningFunction
    void OnOperationAllowanceChanged(WarningFunctionBase* function, bool allow);

signals:
    void NewWarningFunctionAdded(WarningFunctionBase* function);
    void WarningFunctionAboutToBeRemoved(WarningFunctionBase* function);
    void OperationAllowanceChanged(bool allow);

private:
    std::vector< WarningFunctionBase* > m_Functions;

    WarningFunctionBase* m_CurrentRunningFunction;
};

#endif // ! WARNING_MANAGER_H