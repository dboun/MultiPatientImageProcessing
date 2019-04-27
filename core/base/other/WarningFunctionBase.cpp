#include "WarningFunctionBase.h"

#include <QDebug>

WarningFunctionBase::WarningFunctionBase(QObject* parent) : 
    ObserverBase(parent)
{
    this->SetName("Warning Function"); // This needs to be set this manually later
}

WarningFunctionBase::~WarningFunctionBase()
{

}

bool WarningFunctionBase::IsOperationAllowed()
{
    return m_OperationAllowed;
}

QString WarningFunctionBase::GetErrorMessageIfNotAllowed()
{
    if (m_OperationAllowed) 
    {
        m_ErrorMessage = "";
        return "";
    }
    return m_ErrorMessage;
}

void WarningFunctionBase::SetOperationAllowed(bool allowed, 
    QString errorMessageIfNotAllowed)
{
    if (allowed) { errorMessageIfNotAllowed = ""; }

    qDebug() << "WarningFunctionBase::SetOperationAllowed" << this->GetName();
    qDebug() << "WarningFunctionBase::SetOperationAllowed" 
             << ((allowed)?"Allowed":"Not allowed") << errorMessageIfNotAllowed;

    if (errorMessageIfNotAllowed != m_ErrorMessage)
    {
        QString oldErrorMessage = m_ErrorMessage;
        m_ErrorMessage = errorMessageIfNotAllowed;

        if (oldErrorMessage != "")
        {
            qDebug() << "Emit WarningFunctionBase::ErrorWasRemoved";
            emit ErrorMessageWasRemoved(this, oldErrorMessage);
        }

        if (errorMessageIfNotAllowed != "")
        {
            qDebug() << "Emit WarningFunctionBase::NewError";
            emit NewErrorMessage(this, errorMessageIfNotAllowed);
        }
    }

    if (m_OperationAllowed != allowed)
    {
        m_OperationAllowed = allowed;
        qDebug() << "Emit WarningFunctionBase::OperationAllowanceChanged";
        emit OperationAllowanceChanged(this, m_OperationAllowed);
    }
}

void WarningFunctionBase::UpdateWarnings(QStringList newWarnings)
{
    std::vector<QString> updateAdded, updateRemoved;

    for (const QString& nw : newWarnings)
    {
        if (!m_Warnings.contains(nw))
        {
            updateAdded.push_back(nw);
        }
    }

    for (const QString& ow : m_Warnings)
    {
        if (!newWarnings.contains(ow))
        {
            updateRemoved.push_back(ow);
        }
    }

    m_Warnings = newWarnings;

    for (const QString& w : updateAdded)
    {
        emit NewWarning(this, w);
    }

    for (const QString& w : updateRemoved)
    {
        emit WarningWasRemoved(this, w);
    }
}