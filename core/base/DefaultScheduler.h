#ifndef DEFAULT_SCHEDULER_H
#define DEFAULT_SCHEDULER_H

#include <QThread>
#include <QQueue>

#include <mutex>
#include <thread>
#include <atomic>
#include <map>

#include "SchedulerBase.h"
#include "AlgorithmModuleBase.h"

class DefaultScheduler : public SchedulerBase
{
    Q_OBJECT

public:
    static DefaultScheduler& GetInstance();
	~DefaultScheduler();

    void QueueAlgorithm(AlgorithmModuleBase* algorithmModule) override;
    void ClearQueuedAlgorithms() override;

protected:
    DefaultScheduler() : SchedulerBase() {}

private:
    void StartBackgroundCoordinatorIfNecessary();
    void BackgroundCoordinator();
	void ThreadJob(long tid, AlgorithmModuleBase* algorithmModule);
    void AddThreadToJoinQueue(long tid);

    bool IsAlgorithmAllowedToRunYet(AlgorithmModuleBase::SEVERITY severity);

    std::mutex m_Mutex;
	std::atomic<bool> m_CoordinatorRunning {false};
	std::thread m_BackgroundCoordinator;
    std::map< long, std::thread > m_Threads; // key is thread unique id
    QQueue<long> m_ThreadsToJoin;
    long tidNextToGive = 0;

    std::atomic<int> m_NumberOfLowPriorityRunning {0};
    std::atomic<int> m_NumberOfMediumPriorityRunning {0};
    std::atomic<int> m_NumberOfHighPriorityRunning {0};

    const int m_NumberOfAllowedLowPriorityJobs = 
        (QThread::idealThreadCount() / 4 >= 1) ?
            QThread::idealThreadCount() / 4 :
            1;

    const int m_NumberOfAllowedMediumPriorityJobs = 
        (QThread::idealThreadCount() / 2 >= 1) ?
            QThread::idealThreadCount() / 2 :
            1;

    const int m_NumberOfAllowedHighPriorityJobs = 
        QThread::idealThreadCount();
};

#endif // ! DEFAULT_SCHEDULER_H