#include "WarningManager.h"

#include <algorithm>

WarningManager::WarningManager(QObject* parent) : QObject(parent)
{
    
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

void WarningManager::RegisterWarningFunction(WarningFunctionBase* function)
{
    if (!function) { return; }

    if (std::find(m_Functions.begin(), m_Functions.end(), function) == m_Functions.end()) 
    {
        // m_Functions does not contain function
        m_Functions.push_back(function);
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

void WarningManager::OnOperationAllowanceChanged(WarningFunctionBase* function, 
    bool allow)
{
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