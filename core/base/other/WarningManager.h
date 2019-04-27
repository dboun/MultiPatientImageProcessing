#ifndef WARNING_MANAGER_H
#define WARNING_MANAGER_H

#include <QObject>

#include <vector>
#include <map>

#include "WarningFunctionBase.h"

class WarningManager : public QObject
{
    Q_OBJECT

public:
    WarningManager(QObject* parent = nullptr);

    ~WarningManager();

    /** The WarningManager doesn't check if any have are null.
     *  GetFunctions might be unsafe. */
    std::vector< WarningFunctionBase* > GetFunctions();

    /** Returns true if the operation is allowed by all registered functions */
    bool IsOperationAllowed();

    QStringList GetAllErrorMessages();

    void RegisterWarningFunction(WarningFunctionBase* function);
    void UnregisterWarningFunction(WarningFunctionBase* function);
    void UnregisterAllWarningFunctionsWithName(QString functionName);

public slots:
    /** Override these functions to potentially add more functionality */
    virtual void OnNewWarningFunctionAdded(WarningFunctionBase* function);
    virtual void OnWarningFunctionAboutToBeRemoved(WarningFunctionBase* function);

    // Slots for WarningFunction
    void OnOperationAllowanceChanged(WarningFunctionBase* function, bool allow);
    void OnNewErrorMessage(WarningFunctionBase* function, QString errorMessage);
    void OnErrorMessageWasRemoved(WarningFunctionBase* function, QString errorMessageThatWasRemoved);
    void OnNewWarning(WarningFunctionBase* function, QString warning);
    void OnWarningWasRemoved(WarningFunctionBase* function, QString warningThatWasRemoved);

signals:
    void OperationAllowanceChanged(bool allow);
    void NewErrorMessage(QString errorMessage);
    void ErrorMessageWasRemoved(QString errorMessage);

    void NewWarning(QString warning);
    void WarningWasRemoved(QString warningThatWasRemoved);

    void NewWarningFunctionAdded(WarningFunctionBase* function);
    void WarningFunctionAboutToBeRemoved(WarningFunctionBase* function);

private:
    template<class T, class U>
	std::vector<T> IdsOfMap(std::map<T, U>& map)
	{
		std::vector<T> keys;
		keys.reserve(map.size());
		for (auto const& imap : map) {
			keys.push_back(imap.first);
		}
		return keys;
	}

    std::vector< WarningFunctionBase* > m_Functions;

    // The map always has at least an empty QString() set 
    QStringList m_ErrorMessages;
    QStringList m_WarningMessages;

    WarningFunctionBase* m_CurrentRunningFunction;
};

#endif // ! WARNING_MANAGER_H