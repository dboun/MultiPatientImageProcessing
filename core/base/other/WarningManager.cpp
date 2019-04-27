#include "WarningManager.h"

#include <QDebug>

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
    return m_ErrorMessages;
}

void WarningManager::RegisterWarningFunction(WarningFunctionBase* function)
{
    if (!function) { return; }

    if (std::find(m_Functions.begin(), m_Functions.end(), function) == m_Functions.end()) 
    {
        // m_Functions does not contain function
        m_Functions.push_back(function);

        // Connect signals/slots
        connect(function, SIGNAL(OperationAllowanceChanged(WarningFunctionBase*, bool, QString)),
            this, SLOT(OnOperationAllowanceChanged(WarningFunctionBase*, bool, QString))
        );
        connect(function, SIGNAL(NewErrorMessage(WarningFunctionBase*, QString)), 
            this, SLOT(OnNewErrorMessage(WarningFunctionBase*, QString))
        );
        connect(function, SIGNAL(ErrorMessageWasRemoved(WarningFunctionBase*, QString)), 
            this, SLOT(OnErrorMessageWasRemoved(WarningFunctionBase*, QString))
        );
        connect(function, SIGNAL(NewWarning(WarningFunctionBase*, QString)), 
            this, SLOT(OnNewWarning(WarningFunctionBase*, QString))
        );
        connect(function, SIGNAL(WarningWasRemoved(WarningFunctionBase*, QString)), 
            this, SLOT(OnWarningWasRemoved(WarningFunctionBase*, QString))
        );

        emit NewWarningFunctionAdded(function);
    }
}

void WarningManager::UnregisterWarningFunction(WarningFunctionBase* function)
{
    if (!function) { return; }

    qDebug() << "WarningManager::UnregisterWarningFunction" << function->GetName();

    if (std::find(m_Functions.begin(), m_Functions.end(), function) != m_Functions.end()) 
    {
        // m_Functions contains function
        emit WarningFunctionAboutToBeRemoved(function);
        m_Functions.erase(
            std::remove(m_Functions.begin(), m_Functions.end(), function), 
            m_Functions.end()
        );
        
        if (function->GetErrorMessageIfNotAllowed() != "")
        {
            m_ErrorMessages.removeOne(function->GetErrorMessageIfNotAllowed());
        }

        // Disonnect signals/slots
        disconnect(function, SIGNAL(OperationAllowanceChanged(WarningFunctionBase*, bool, QString)),
            this, SLOT(OnOperationAllowanceChanged(WarningFunctionBase*, bool, QString))
        );
        disconnect(function, SIGNAL(NewErrorMessage(WarningFunctionBase*, QString)), 
            this, SLOT(OnNewErrorMessage(WarningFunctionBase*, QString))
        );
        disconnect(function, SIGNAL(ErrorMessageWasRemoved(WarningFunctionBase*, QString)), 
            this, SLOT(OnErrorMessageWasRemoved(WarningFunctionBase*, QString))
        );
        disconnect(function, SIGNAL(NewWarning(WarningFunctionBase*, QString)), 
            this, SLOT(OnNewWarning(WarningFunctionBase*, QString))
        );
        disconnect(function, SIGNAL(WarningWasRemoved(WarningFunctionBase*, QString)), 
            this, SLOT(OnWarningWasRemoved(WarningFunctionBase*, QString))
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

void WarningManager::OnOperationAllowanceChanged(WarningFunctionBase* function, bool allow)
{
    qDebug() << "WarningManager::OnOperationAllowanceChanged" << function->GetName();
    qDebug() << "WarningManager::OnOperationAllowanceChanged" 
             << ((allow)?"Allowed":"Not allowed");
    
    // See if another function is false
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

    if (allow && !foundAnotherFalse)
    {
        emit OperationAllowanceChanged(true);
    }
    if (!allow && !foundAnotherFalse) 
    {
        emit OperationAllowanceChanged(false);
    }
}

void WarningManager::OnNewErrorMessage(WarningFunctionBase* function, QString errorMessage)
{
    if (!m_ErrorMessages.contains(errorMessage))
    {
        m_ErrorMessages.push_back(errorMessage);
        emit NewErrorMessage(errorMessage);
    }
}

void WarningManager::OnErrorMessageWasRemoved(WarningFunctionBase* function, 
    QString errorMessageThatWasRemoved)
{
    if (m_ErrorMessages.contains(errorMessageThatWasRemoved))
    {
        m_ErrorMessages.removeOne(errorMessageThatWasRemoved);
        emit ErrorMessageWasRemoved(errorMessageThatWasRemoved);
    }
}

void WarningManager::OnNewWarning(WarningFunctionBase* function, QString warning)
{
    if (!m_WarningMessages.contains(warning))
    {
        m_WarningMessages.push_back(warning);
        qDebug() << "Emit WarningManager::NewWarning" << warning;
        emit NewWarning(warning);
    }
}

void WarningManager::OnWarningWasRemoved(WarningFunctionBase* function, 
    QString warningThatWasRemoved)
{
    if (m_WarningMessages.contains(warningThatWasRemoved))
    {
        m_WarningMessages.removeOne(warningThatWasRemoved);
        qDebug() << "Emit WarningManager::WarningWasRemoved" << warningThatWasRemoved;
        emit WarningWasRemoved(warningThatWasRemoved);
    }
}