#ifndef SCHEDULER_BASE_H
#define SCHEDULER_BASE_H

#include <QObject>
#include <QThread>
#include <QQueue>

#include <thread>
#include <mutex>
#include <atomic>

#include "AlgorithmModuleBase.h"

class SchedulerBase : public QObject
{
	Q_OBJECT

public:
    static SchedulerBase& GetInstance();

	~SchedulerBase();

    /** Override these methods to provide functionality */
	virtual void QueueAlgorithm(AlgorithmModuleBase* algorithmModule);
    virtual bool IsSafeToExit();
    virtual void ClearQueuedAlgorithms(); // Override to make thread safe

signals:
	void JobFinished(AlgorithmModuleBase* algorithmModule);

protected:
    /** Increment/Decrement are thread safe */
    void IncrementRunningAlgorithms();
    void DecrementRunningAlgorithms();

    /** Queue operations (not thread safe) */
    bool IsQueueEmpty();
    void ClearQueue();
    void AddToQueue(AlgorithmModuleBase* algorithmModule);
    AlgorithmModuleBase* PeekQueueHeadWithoutPop();
    AlgorithmModuleBase* GetQueueHeadAndPop();

	QQueue< AlgorithmModuleBase* > m_Queue;

    std::atomic<int> m_RunningAlgorithms {0};

    SchedulerBase() : QObject(nullptr) {}
};

#endif // ! SCHEDULER_BASE_H