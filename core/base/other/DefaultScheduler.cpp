#include "DefaultScheduler.h"

#include <QDebug>
#include <QThread>
#include <memory>

DefaultScheduler* DefaultScheduler::GetInstance()
{
	// static is initialized only once
    static std::shared_ptr<DefaultScheduler> instance = std::shared_ptr<DefaultScheduler>(
		new DefaultScheduler()
	); 
    return instance.get();
}

DefaultScheduler::~DefaultScheduler()
{
	// Clean up
	while (!m_ThreadsToJoin.empty())
	{
		m_Threads[m_ThreadsToJoin.head()].join();
		m_ThreadsToJoin.dequeue();
	}

	// Clean up coordinator
	if (m_BackgroundCoordinator.joinable()) { m_BackgroundCoordinator.join(); }
}

void DefaultScheduler::QueueAlgorithm(AlgorithmModuleBase* algorithmModule)
{
	std::lock_guard<std::mutex> lg(m_Mutex);
	IncrementRunningAlgorithms();
	AddToQueue(algorithmModule);
	StartBackgroundCoordinatorIfNecessary();
}

void DefaultScheduler::ClearQueuedAlgorithms()
{
	std::lock_guard<std::mutex> lg(m_Mutex);
	ClearQueue();
}

DefaultScheduler::DefaultScheduler() : SchedulerBase()
{
	qDebug() << "DefaultScheduler: Ideal thread count in system:" << QThread::idealThreadCount();
	qDebug() << "DefaultScheduler: Allowed low severity:        " << m_NumberOfAllowedLowSeverityJobs;
	qDebug() << "DefaultScheduler: Allowed medium severity:     " << m_NumberOfAllowedMediumSeverityJobs;
	qDebug() << "DefaultScheduler: Allowed high severity:       " << m_NumberOfAllowedHighSeverityJobs; 
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
	std::unique_lock<std::mutex> ul(m_Mutex);
	m_CoordinatorRunning = true;

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
		// Put the algorithm to the last position
		GetQueueHeadAndPop(); // pop
		bool isOnlyOneAlgorithm = IsQueueEmpty(); // find if it's the only one on the queue
		                                          // can't use this right now, deadlock
		AddToQueue(a); // push to last position

		ul.unlock();
		QThread::msleep(500);
		ul.lock();

		if (IsQueueEmpty())
		{
			m_CoordinatorRunning = false;
			ul.unlock();
			return;
		}

		a = PeekQueueHeadWithoutPop();
	}

	a = GetQueueHeadAndPop();

	m_Threads[tidNextToGive] = std::thread(&DefaultScheduler::ThreadJob, this,
		tidNextToGive, a
	);
	tidNextToGive++;

	bool restart = false;
	if (!IsQueueEmpty()) { restart = true; }

	m_CoordinatorRunning = false;
	ul.unlock();

	// It's ok if it somehow runs twice
	// because of the mutex
	if (restart) { this->BackgroundCoordinator(); } 
}

void DefaultScheduler::ThreadJob(long tid, AlgorithmModuleBase* algorithmModule)
{
	this->IncrementRunningWithThisSeverityCounter(algorithmModule->GetSeverity());
	algorithmModule->Run();
	emit JobFinished(algorithmModule);
	AddThreadToJoinQueue(tid);
	this->DecrementRunningWithThisSeverityCounter(algorithmModule->GetSeverity());
	DecrementRunningAlgorithms();
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
			qDebug() << "IsAlgorithmAllowedToRunYet: Running for low severity" 
			         << QString::number(m_NumberOfLowSeverityRunning);
			return (m_NumberOfLowSeverityRunning < m_NumberOfAllowedLowSeverityJobs);
		case AlgorithmModuleBase::SEVERITY::MEDIUM:
			qDebug() << "IsAlgorithmAllowedToRunYet: Running for medium severity" 
			         << QString::number(m_NumberOfMediumSeverityRunning);
			return (m_NumberOfMediumSeverityRunning < m_NumberOfAllowedMediumSeverityJobs);
		case AlgorithmModuleBase::SEVERITY::HIGH:
			qDebug() << "IsAlgorithmAllowedToRunYet: Running for high severity" 
			         << QString::number(m_NumberOfHighSeverityRunning);
			return (m_NumberOfHighSeverityRunning < m_NumberOfAllowedHighSeverityJobs);
	}
}

void DefaultScheduler::IncrementRunningWithThisSeverityCounter(AlgorithmModuleBase::SEVERITY severity)
{
	switch (severity)
	{
		case AlgorithmModuleBase::SEVERITY::LOW:
			qDebug() << "Incrementing running";
			m_NumberOfLowSeverityRunning++;
			break;
		case AlgorithmModuleBase::SEVERITY::MEDIUM:
			qDebug() << "Incrementing running";
			m_NumberOfMediumSeverityRunning++;
			break;
		case AlgorithmModuleBase::SEVERITY::HIGH:
			qDebug() << "Incrementing running" << m_NumberOfHighSeverityRunning;
			m_NumberOfHighSeverityRunning++;
			qDebug() << m_NumberOfHighSeverityRunning;
			break;
	}
}

void DefaultScheduler::DecrementRunningWithThisSeverityCounter(AlgorithmModuleBase::SEVERITY severity)
{
	switch (severity)
	{
		case AlgorithmModuleBase::SEVERITY::LOW:
			qDebug() << "Decrementing running";
			m_NumberOfLowSeverityRunning--;
			break;
		case AlgorithmModuleBase::SEVERITY::MEDIUM:
			qDebug() << "Decrementing running";
			m_NumberOfMediumSeverityRunning--;
			break;
		case AlgorithmModuleBase::SEVERITY::HIGH:
			qDebug() << "Decrementing running";
			m_NumberOfHighSeverityRunning--;
			break;
	}
}
