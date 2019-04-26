#ifndef WARNING_FUNCTION_BASE_H
#define WARNING_FUNCTION_BASE_H

#include <QString>
#include <QStringList>

#include "ObserverBase.h"

/** This class is meant to be used for functions before running an algorithm.
 *  It is to be used as a warning/guard system, to check if all the creteria for running
 *  an algorithm are met. */
class WarningFunctionBase : public ObserverBase
{
    Q_OBJECT

public:
    WarningFunctionBase(QObject* parent = nullptr);

    ~WarningFunctionBase();

    /** This is provided, but it's best to listen for OperationAllowanceChanged */
    bool IsOperationAllowed();

    QString GetErrorMessageIfNotAllowed();

signals:
    void OperationAllowanceChanged(WarningFunctionBase* function, bool allow,
        QString errorMessageIfNotAllowed
    );
    void NewWarning(WarningFunctionBase* function, QString warning);
    void WarningWasRemoved(WarningFunctionBase* function, QString warningThatWasRemoved);

protected:
    // Implementations of this class, should call the below methods to update their state
    void SetOperationAllowed(bool allowed,
        QString GetErrorMessageIfNotAllowed = ""
    );
    /** This was call WarningWasRemoved if an old warning isn't in newWarnings */
    void UpdateWarnings(QStringList newWarnings);

private:
    QString                m_ErrorMessage;
    QStringList            m_Warnings;
    bool                   m_OperationAllowed = true;
};

#endif // ! WARNING_FUNCTION_BASE_H