#include "AlgorithmModuleBase.h"

#include <QDebug>

void AlgorithmModuleBase::SetDataManager(DataManager* dataManager)
{
    m_DataManager = dataManager;
}

void AlgorithmModuleBase::SetDataView(DataViewBase* dataView)
{
    m_DataView = dataView;

    connect(this, SIGNAL(ProgressUpdateUI(long, QString, int)),
        dataView, SLOT(UpdateProgressHandler(long, QString, int))
    );
}

void AlgorithmModuleBase::SetUid(long uid)
{
    m_Uid = uid;
}

long AlgorithmModuleBase::GetUid()
{
    return m_Uid;
}

QString AlgorithmModuleBase::GetAlgorithmName()
{
    return m_AlgorithmName;
}

QString AlgorithmModuleBase::GetAlgorithmNameShort()
{
    return m_AlgorithmNameShort;
}

AlgorithmModuleBase::SEVERITY AlgorithmModuleBase::GetSeverity()
{
    return m_Severity;
}

bool AlgorithmModuleBase::IsRunning()
{
    return m_IsRunning;
}

bool AlgorithmModuleBase::IsCanceled()
{
    return m_CancelFlag;
}

void AlgorithmModuleBase::Run()
{
    qDebug() << "Algorithm module" << m_AlgorithmName << "Running";
    
    if (m_IsRunning)
    {
        emit AlgorithmFinishedWithError(this, 
            QString("This instance of ") +
            m_AlgorithmNameShort + QString(" is already running")
        );
        return;
    }

    if (m_CancelFlag)
    {
        m_CancelFlag = false;
        return;
    }

    m_IsRunning = true;

    Algorithm();

    m_IsRunning = false;
    m_CancelFlag = false;
}

void AlgorithmModuleBase::Algorithm()
{
    emit ProgressUpdateUI(m_Uid, QString("Test Progress"), 100);

    // There should be regular checks in 
    // algorithm modules that inherit this
    // to see if m_CancelFlag has been set to true
}

DataManager* AlgorithmModuleBase::GetDataManager()
{
    return m_DataManager;
}

DataViewBase* AlgorithmModuleBase::GetDataView()
{
    return m_DataView;
}

void AlgorithmModuleBase::SetAlgorithmName(QString algorithmName)
{
    m_AlgorithmName = algorithmName;
}

void AlgorithmModuleBase::SetAlgorithmNameShort(QString algorithmNameShort)
{
    m_AlgorithmNameShort = algorithmNameShort;
}