#include "WarningManager.h"

#include <algorithm>

WarningManager::WarningManager(QObject* parent) : QObject(parent)
{
    connect(this, SIGNAL(NewWarningFunctionAdded(WarningFunctionBase*)),
        this, SLOT(OnNewWarningFunctionAdded(WarningFunctionBase*))
    );
    connect(this, SIGNAL(WarningFunctionAboutToBeRemoved(WarningFunctionBase*)),
        this, SLOT(OnWarningFunctionAboutToBeRemoved(WarningFunctionBase*))
    );
}

WarningManager::~WarningManager()
{
    
}

std::vector< WarningFunctionBase* > WarningManager::GetFunctions()
{
    return m_Functions;
}

bool WarningManager::IsOperationAllowed()
{
    for (auto* function : m_Functions)
    {
        if (!function) { continue; }
        if (!function->IsOperationAllowed()) { return false; }
    }
    return true;
}

QStringList WarningManager::GetAllErrorMessages()
{
    QStringList errorMessages;
    for (auto* function : IdsOfMap<WarningFunctionBase*, QString>(m_ErrorMessageMap))
    {
        if (!function) { continue; }
        if (m_ErrorMessageMap[function] != "")
        {
            errorMessages.push_back(m_ErrorMessageMap[function]);
        }
    }
    return errorMessages;
}

void WarningManager::RegisterWarningFunction(WarningFunctionBase* function)
{
    if (!function) { return; }

    if (std::find(m_Functions.begin(), m_Functions.end(), function) == m_Functions.end()) 
    {
        // m_Functions does not contain function
        m_Functions.push_back(function);
        m_ErrorMessageMap[function] = QString();
        emit NewWarningFunctionAdded(function);
    }
}

void WarningManager::UnregisterWarningFunction(WarningFunctionBase* function)
{
    if (!function) { return; }

    if (std::find(m_Functions.begin(), m_Functions.end(), function) != m_Functions.end()) 
    {
        // m_Functions contains function
        emit WarningFunctionAboutToBeRemoved(function);
        m_Functions.erase(
            std::remove(m_Functions.begin(), m_Functions.end(), function), 
            m_Functions.end()
        );
        std::map<WarningFunctionBase*, QString>::iterator eIter = m_ErrorMessageMap.find(function);
        if( eIter != m_ErrorMessageMap.end() ) {
            m_ErrorMessageMap.erase( eIter );
        }
    }
}

void WarningManager::UnregisterAllWarningFunctionsWithName(QString functionName)
{
    std::vector< WarningFunctionBase* > functionsToRemove;

    for (auto* function : m_Functions)
    {
        if (!function) { continue; } // Removing it is weird

        if (function->GetName() == functionName)
        {
            functionsToRemove.push_back(function);
        }
    }

    for (auto* function : functionsToRemove)
    {
        this->UnregisterWarningFunction(function);
    }
}

void WarningManager::OnNewWarningFunctionAdded(WarningFunctionBase* function)
{

}

void WarningManager::OnWarningFunctionAboutToBeRemoved(WarningFunctionBase* function)
{

}

void WarningManager::OnOperationAllowanceChanged(WarningFunctionBase* function, bool allow,
        QString errorMessageIfNotAllowed)
{
    QString oldErrorMessage = m_ErrorMessageMap[function];
    m_ErrorMessageMap[function] = errorMessageIfNotAllowed;
    
    if (oldErrorMessage != "") 
    {  
        emit ErrorMessageWasRemoved(oldErrorMessage);
    }
    else {
        emit NewErrorMessage(errorMessageIfNotAllowed);
    }
    
    if (allow)
    {
        // It couldn't have been allowed before, because this function was false
        emit OperationAllowanceChanged(true);
    }
    else {
        // Another function might be false too, so the allowance hasn't necessarily changed
        bool foundAnotherFalse = false;
        for (auto* f : m_Functions)
        {
            if (!f) { continue; }
            if (!f->IsOperationAllowed() && f != function)
            {
                foundAnotherFalse = true;
                break;
            }
        }

        if (!foundAnotherFalse)
        {
            emit OperationAllowanceChanged(false);
        }
    }
}

void WarningManager::OnNewWarning(WarningFunctionBase* function, QString warning)
{
    if (!m_WarningMessages.contains(warning))
    {
        m_WarningMessages.push_back(warning);
        emit NewWarning(warning);
    }
}

void WarningManager::OnWarningWasRemoved(WarningFunctionBase* function, 
    QString warningThatWasRemoved)
{
    if (m_WarningMessages.contains(warningThatWasRemoved))
    {
        m_WarningMessages.removeAll(warningThatWasRemoved);
        emit WarningWasRemoved(warningThatWasRemoved);
    }
}