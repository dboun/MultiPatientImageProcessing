#ifndef DEFAULT_SCHEDULER_H
#define DEFAULT_SCHEDULER_H

#include <QThread>

#include <mutex>
#include <thread>
#include <atomic>

#include "SchedulerBase.h"
#include "AlgorithmModuleBase.h"

class DefaultScheduler : public SchedulerBase
{
    Q_OBJECT

public:
	DefaultScheduler(QObject *parent = nullptr);
	~DefaultScheduler() {}

private:
    void QueueAlgorithm(AlgorithmModuleBase* algorithmModule) override;
    void ClearQueuedAlgorithms() override;

    void StartBackgroundCoordinatorIfNecessary();
    void BackgroundCoordinator();
	void ThreadJob(AlgorithmModuleBase* algorithmModule);

    std::mutex m_Mutex;
	bool m_CoordinatorRunning = false;
	std::thread m_BackgroundCoordinator;

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

    const int m_NumberOfAllowedLowPriorityJobs = 
        QThread::idealThreadCount();
};

#endif // ! DEFAULT_SCHEDULER_H