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

        // Connect signals/slots
        bool ok = connect(function, SIGNAL(OperationAllowanceChanged(WarningFunctionBase*, bool, QString)),
            this, SLOT(OnOperationAllowanceChanged(WarningFunctionBase*, bool, QString))
        );
        if (ok) { qDebug() << "ok"; }
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
        std::map<WarningFunctionBase*, QString>::iterator eIter = m_ErrorMessageMap.find(function);
        if( eIter != m_ErrorMessageMap.end() ) {
            m_ErrorMessageMap.erase( eIter );
        }

        // Disonnect signals/slots
        disconnect(function, SIGNAL(OperationAllowanceChanged(WarningFunctionBase*, bool, QString)),
            this, SLOT(OnOperationAllowanceChanged(WarningFunctionBase*, bool, QString))
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

void WarningManager::OnOperationAllowanceChanged(WarningFunctionBase* function, bool allow,
        QString errorMessageIfNotAllowed)
{
    qDebug() << "WarningManager::OnOperationAllowanceChanged" << function->GetName();
    qDebug() << "WarningManager::OnOperationAllowanceChanged" 
             << ((allow)?"Allowed":"Not allowed") << errorMessageIfNotAllowed;

    QString oldErrorMessage = m_ErrorMessageMap[function];
    m_ErrorMessageMap[function] = errorMessageIfNotAllowed;
    
    if (oldErrorMessage != "") 
    {  
        qDebug() << "emit WarningManager::ErrorMessageWasRemoved" << oldErrorMessage;
        emit ErrorMessageWasRemoved(oldErrorMessage);
    }
    else {
        if (!allow)
        {
            qDebug() << "emit WarningManager::NewErrorMessage" << errorMessageIfNotAllowed;
            emit NewErrorMessage(errorMessageIfNotAllowed);
        }
    }
    
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
        m_WarningMessages.removeAll(warningThatWasRemoved);
        qDebug() << "Emit WarningManager::WarningWasRemoved" << warningThatWasRemoved;
        emit WarningWasRemoved(warningThatWasRemoved);
    }
}