#include "AlgorithmModuleBase.h"

#include <QDebug>
#include <QThread>

AlgorithmModuleBase::AlgorithmModuleBase(QObject *parent) : QObject(parent)
{
    
}

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

void AlgorithmModuleBase::SetAppName(QString appName)
{
    m_AppName = appName;
}
void AlgorithmModuleBase::SetAppNameShort(QString appNameShort)
{
    m_AppNameShort = appNameShort;
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

    if(!m_DataManager)
    {
        emit AlgorithmFinishedWithError(this, 
            m_AlgorithmNameShort + QString("No DataManager set")
        );
    }
    
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
    // For debugging
    emit ProgressUpdateUI(m_Uid, QString("Test Progress Start"), 0);
    QThread::sleep(2);
    emit ProgressUpdateUI(m_Uid, QString("Test Progress Start"), 20);
    if (m_CancelFlag) { return; }

    QThread::sleep(5);    
    emit ProgressUpdateUI(m_Uid, QString("Test Progress Start"), 70);
    if (m_CancelFlag) { return; }
    QThread::sleep(3);
    emit ProgressUpdateUI(m_Uid, QString("Test Progress Start"), 100);
    emit AlgorithmFinished(this);
    

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

void AlgorithmModuleBase::SetSeverity(SEVERITY severity)
{
    m_Severity = severity;
}

QString AlgorithmModuleBase::GetAppName()
{
    return m_AppName;
}

QString AlgorithmModuleBase::GetAppNameShort()
{
    return m_AppNameShort;
}