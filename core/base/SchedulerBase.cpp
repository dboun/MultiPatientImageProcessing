#include "SchedulerBase.h"

#include <QDebug>

SchedulerBase& SchedulerBase::GetInstance()
{
    static SchedulerBase instance; // static is initialized only once
    return instance;
}

SchedulerBase::~SchedulerBase()
{
    ClearQueuedAlgorithms();
    
    /** Just a fail safe. 
     * Hopefully, there would not be a need
     * for this, if IsSafeToExit() is queried
     * when 'X' is clicked */
    while(m_RunningAlgorithms > 0)
    {
        QThread::sleep(1);
    }
}

void SchedulerBase::QueueAlgorithm(AlgorithmModuleBase* algorithmModule)
{
    // This is for debugging (and will hang the main thread with an actual algorithm)
    // It shows some general steps though, without threads
    
    AddToQueue(algorithmModule);
    // When the time is right, pop 
    AlgorithmModuleBase* a = GetQueueHeadAndPop();
    // Run
    IncrementRunningAlgorithms();
    a->Run();
    DecrementRunningAlgorithms();
    // Emit signal for the UI
    // (The algorithm will have put it's files in DataManager beforehand)
    emit JobFinished(a);
}

bool SchedulerBase::IsSafeToExit()
{
    if (m_RunningAlgorithms > 0) { return false; }
    else { return true; }
}

void SchedulerBase::ClearQueuedAlgorithms()
{
    // An actual implementation will maybe need to be thread safe
    ClearQueue();
}

void SchedulerBase::IncrementRunningAlgorithms()
{
    m_RunningAlgorithms++;
}

void SchedulerBase::DecrementRunningAlgorithms()
{
    m_RunningAlgorithms--;
}

bool SchedulerBase::IsQueueEmpty()
{
    return m_Queue.empty();
}

void SchedulerBase::ClearQueue()
{
    m_Queue.clear();
}

void SchedulerBase::AddToQueue(AlgorithmModuleBase* algorithmModule)
{
    m_Queue.enqueue(algorithmModule);
}

AlgorithmModuleBase* SchedulerBase::PeekQueueHeadWithoutPop()
{
    return m_Queue.head();
}

AlgorithmModuleBase* SchedulerBase::GetQueueHeadAndPop()
{
    AlgorithmModuleBase* a = m_Queue.head();
    m_Queue.dequeue();
    return a;
}