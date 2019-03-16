#include "DefaultScheduler.h"

#include <QDebug>
#include <QThread>

DefaultScheduler::DefaultScheduler(QObject *parent) : SchedulerBase(parent)
{
	qDebug() << "DefaultScheduler::DefaultScheduler()";
}

void DefaultScheduler::QueueAlgorithm(AlgorithmModuleBase* algorithmModule)
{
	std::lock_guard<std::mutex> lg(m_Mutex);
	AddToQueue(algorithmModule);
	StartBackgroundCoordinatorIfNecessary();
}

void DefaultScheduler::ClearQueuedAlgorithms()
{
	std::lock_guard<std::mutex> lg(m_Mutex);
	ClearQueue();
}

void DefaultScheduler::StartBackgroundCoordinatorIfNecessary()
{
	if (!m_CoordinatorRunning)
	{
		m_CoordinatorRunning = true;
		if (m_BackgroundCoordinator.joinable()) { m_BackgroundCoordinator.join(); }
		m_BackgroundCoordinator = std::thread(&DefaultScheduler::BackgroundCoordinator, this);
	}
}

void DefaultScheduler::BackgroundCoordinator()
{
	m_CoordinatorRunning = true;
	std::unique_lock<std::mutex> ul(m_Mutex);

	// Clean up
	while (!m_ThreadsToJoin.empty())
	{
		m_Threads[m_ThreadsToJoin.head()].join();
		m_ThreadsToJoin.dequeue();
	}
	
	// See if there is nothing to do
	if (IsQueueEmpty())
	{
		ul.unlock();
		m_CoordinatorRunning = false;
		return;
	}
	
	AlgorithmModuleBase* a = PeekQueueHeadWithoutPop();

	while (!IsAlgorithmAllowedToRunYet(a->GetSeverity()))
	{
		ul.unlock();
		QThread::msleep(500);
		ul.lock();
		a = PeekQueueHeadWithoutPop();
	}

	a = GetQueueHeadAndPop();
	ul.unlock();

	m_Threads[tidNextToGive] = std::thread(&DefaultScheduler::ThreadJob, this,
		tidNextToGive, a
	);
	tidNextToGive++;
	m_CoordinatorRunning = false;
}

void DefaultScheduler::ThreadJob(long tid, AlgorithmModuleBase* algorithmModule)
{
	algorithmModule->Run();
	emit JobFinished(algorithmModule);
	AddThreadToJoinQueue(tid);
}

void DefaultScheduler::AddThreadToJoinQueue(long tid)
{
	std::unique_lock<std::mutex> ul(m_Mutex);
	m_ThreadsToJoin.enqueue(tid);
}

bool DefaultScheduler::IsAlgorithmAllowedToRunYet(AlgorithmModuleBase::SEVERITY severity)
{
	switch (severity)
	{
		case AlgorithmModuleBase::SEVERITY::LOW:
			return m_NumberOfLowPriorityRunning < m_NumberOfAllowedLowPriorityJobs;
		case AlgorithmModuleBase::SEVERITY::MEDIUM:
			return m_NumberOfMediumPriorityRunning < m_NumberOfAllowedMediumPriorityJobs;
		case AlgorithmModuleBase::SEVERITY::HIGH:
			return m_NumberOfHighPriorityRunning < m_NumberOfAllowedHighPriorityJobs;
	}
}