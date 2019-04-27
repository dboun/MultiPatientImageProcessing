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
    static DefaultScheduler* GetInstance();
	~DefaultScheduler();

    void QueueAlgorithm(AlgorithmModuleBase* algorithmModule) override;
    void ClearQueuedAlgorithms() override;

protected:
    DefaultScheduler();

private:
    void StartBackgroundCoordinatorIfNecessary();
    void BackgroundCoordinator();
	void ThreadJob(long tid, AlgorithmModuleBase* algorithmModule);
    void AddThreadToJoinQueue(long tid);

    bool IsAlgorithmAllowedToRunYet(AlgorithmModuleBase::SEVERITY severity);

    void IncrementRunningWithThisSeverityCounter(AlgorithmModuleBase::SEVERITY severity);
    void DecrementRunningWithThisSeverityCounter(AlgorithmModuleBase::SEVERITY severity);

    std::mutex m_Mutex;
	std::atomic<bool> m_CoordinatorRunning {false};
	std::thread m_BackgroundCoordinator;
    std::map< long, std::thread > m_Threads; // key is thread unique id
    QQueue<long> m_ThreadsToJoin;
    long tidNextToGive = 0;

    std::atomic<int> m_NumberOfLowSeverityRunning {0};
    std::atomic<int> m_NumberOfMediumSeverityRunning {0};
    std::atomic<int> m_NumberOfHighSeverityRunning {0};

    const int m_NumberOfAllowedLowSeverityJobs = 
        QThread::idealThreadCount() * 2;

    const int m_NumberOfAllowedMediumSeverityJobs = 
        (QThread::idealThreadCount() > 2) ?
        QThread::idealThreadCount() - 1 :
        QThread::idealThreadCount();

    /** Basically: 16core -> 8, 8core -> 4, 6/4core -> 3, 2/1core -> 1 */
    const int m_NumberOfAllowedHighSeverityJobs = 
        (QThread::idealThreadCount() >= 8) ? 
            QThread::idealThreadCount() / 2 :
            (QThread::idealThreadCount() >= 4) ? 
                3 :
                1;
};

#endif // ! DEFAULT_SCHEDULER_H