#include "SchedulerBase.h"

#include "ApplicationBase.h"

SchedulerBase::SchedulerBase(QObject *parent) : QObject(parent)
{
	int threadCount = QThread::idealThreadCount();

	qDebug() << QString("Ideal number of threads: ") << QString::number(threadCount);
	
	threadCount = (threadCount / 2 + 1 < 2) ? 2 : threadCount / 2 + 1;
	qDebug() << QString("Automatically setting max parallel jobs to: ") << QString::number(threadCount);
	
	m_MaxParallelJobs = threadCount;
	//m_MaxParallelJobs = 4;
}

SchedulerBase::~SchedulerBase()
{
	Stop();
}

void SchedulerBase::Start()
{
	std::lock_guard<std::mutex> lg(m_Mutex);
	
	m_StopFlag = false;

	if (m_Data.size() == 0)
	{
		return;
	}

	if (!m_CoordinatorRunning)
	{
		m_CoordinatorRunning = true;
		if (m_BackgroundCoordinator.joinable()) { m_BackgroundCoordinator.join(); }
		m_BackgroundCoordinator = std::thread(&SchedulerBase::BackgroundCoordinator, this);
	}
}

void SchedulerBase::Stop()
{
	std::lock_guard<std::mutex> lg(m_Mutex);

	m_StopFlag = true;
	// TODO: Maybe do something for child threads?

	if (m_BackgroundCoordinator.joinable()) { m_BackgroundCoordinator.join(); }

	m_Data.clear();
}

void SchedulerBase::AddData(std::shared_ptr<SchedulerJobData> data)
{
	std::unique_lock<std::mutex> ul(m_Mutex);
	
	m_Data.push_back(data);

	if (!m_StopFlag && !m_CoordinatorRunning)
	{
		ul.unlock();
		Start();
	}
}

void SchedulerBase::SetDataManager(DataManager* dataManager)
{
	m_DataManager = dataManager;
}

void SchedulerBase::SetMaxParallelJobs(int maxParallelJobs)
{
	m_MaxParallelJobs = maxParallelJobs;
}

void SchedulerBase::progressUpdateFromApplication(long uid, QString message, int progress)
{
	emit updateProgress(uid, progress);
}

void SchedulerBase::BackgroundCoordinator()
{
	qDebug() << QString("(Background coordinator) started");
	m_NumberOfUnfishedJobsThisRound = m_Data[0]->uids.size();

	std::vector<std::thread> threads(m_Data[0]->uids.size());
	int counterForThreadsVec = 0;
	int numberOfOpenThreads = 0;
	int oldestOpenThread = 0;

	for (const auto& uid : m_Data[0]->uids)
	{
		qDebug() << QString("(Background coordinator) Running for uid: ") << QString::number(uid);

		if (numberOfOpenThreads == m_MaxParallelJobs)
		{
			threads[oldestOpenThread].join();
			oldestOpenThread++;
			numberOfOpenThreads--;
		}

		numberOfOpenThreads++;
		threads[counterForThreadsVec++] = std::thread(&SchedulerBase::ThreadJob, this,
			uid, std::ref(m_Data[0]->iids[uid]), 0
		);
	}

	for (int i = oldestOpenThread; i < m_Data[0]->uids.size(); i++) {
		threads[i].join();
	}

	std::lock_guard<std::mutex> lg(m_Mutex);
	m_CoordinatorRunning = false;
	m_Data.erase(m_Data.begin());

	if (!m_StopFlag)
	{
		connect(this, SIGNAL(roundFinished()), this, SLOT(Start()));
		emit roundFinished();
	}
}

void SchedulerBase::ResultFinished(long uid)
{
	std::unique_lock<std::mutex> ul(m_Mutex);
	
	qDebug() << QString("Result finished");

	m_NumberOfUnfishedJobsThisRound--;
	emit jobFinished(uid);
}

void SchedulerBase::ThreadJob(long uid, std::vector<long> &iids, const int customFlag)
{
	// This method is supposed to be overriden
	// Note that by inheriting SchedulerBase and using SetDataManager you can have access to everything
	
	qDebug() << "SchedulerBase::ThreadJob()" << "Running dummy application";
	ApplicationBase app;
	connect(&app, SIGNAL(ProgressUpdateUI(long, QString, int)), this, SLOT(progressUpdateFromApplication(long, QString, int)));
	app.SetUid(uid);
	emit app.EmitProgressUpdateForDebugging();
}