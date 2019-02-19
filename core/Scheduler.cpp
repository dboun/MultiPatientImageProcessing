#include "Scheduler.h"

#include "ApplicationBase.h"

Scheduler::Scheduler(QObject *parent) : QObject(parent)
{
	int threadCount = QThread::idealThreadCount();

	qDebug() << QString("Ideal number of threads: ") << QString::number(threadCount);
	
	threadCount = (threadCount / 2 + 1 < 2) ? 2 : threadCount / 2 + 1;
	qDebug() << QString("Automatically setting max parallel jobs to: ") << QString::number(threadCount);
	
	m_MaxParallelJobs = threadCount;
	//m_MaxParallelJobs = 4;
}

Scheduler::~Scheduler()
{
	Stop();
}

void Scheduler::Start()
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
		m_BackgroundCoordinator = std::thread(&Scheduler::BackgroundCoordinator, this);
	}
}

void Scheduler::Stop()
{
	std::lock_guard<std::mutex> lg(m_Mutex);

	m_StopFlag = true;
	// TODO: Maybe do something for child threads?

	if (m_BackgroundCoordinator.joinable()) { m_BackgroundCoordinator.join(); }

	m_Data.clear();
}

void Scheduler::AddData(std::shared_ptr<Data> data)
{
	std::unique_lock<std::mutex> ul(m_Mutex);
	
	m_Data.push_back(data);

	if (!m_StopFlag && !m_CoordinatorRunning)
	{
		ul.unlock();
		Start();
	}
}

void Scheduler::SetMaxParallelJobs(int maxParallelJobs)
{
	m_MaxParallelJobs = maxParallelJobs;
}

void Scheduler::progressUpdateFromApplication(long uid, QString message, int progress)
{
	emit updateProgress(uid, progress);
}

void Scheduler::BackgroundCoordinator()
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
		m_Data[0]->resultPath[uid] = m_Data[0]->patientDirectoryPath[uid] + std::string("/MPIP_output/labels_res.nii.gz");
		threads[counterForThreadsVec++] = std::thread(&Scheduler::ThreadJob, this,
			uid, std::ref(m_Data[0]->imagesPaths[uid]), 
			std::ref(m_Data[0]->maskPath[uid]), std::ref(m_Data[0]->patientDirectoryPath[uid])
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

void Scheduler::ResultFinished(long uid)
{
	std::unique_lock<std::mutex> ul(m_Mutex);
	
	qDebug() << QString("Result finished");

	m_NumberOfUnfishedJobsThisRound--;
	emit jobFinished(uid);
}

void Scheduler::ThreadJob(long uid, std::vector<std::string> &imagesPaths, std::string &maskPath, std::string &patientDirectoryPath)
{
	qDebug() << QString("Thread started for: ") << QString::number(uid);

#ifdef BUILD_GEODESIC_TRAINING
	ApplicationGeodesicTrainingSegmentation<float, 3> geodesic; // TODO: Support 2D
	geodesic.SetUid(uid);
	connect(&geodesic, SIGNAL(ProgressUpdateUI(long, QString, int)), this, SLOT(progressUpdateFromApplication(long, QString, int)));
	geodesic.SetInputImages(imagesPaths);
	geodesic.SetLabels(maskPath);
	geodesic.SetOutputPath(patientDirectoryPath + std::string("/MPIP_output"));
	geodesic.SetSaveAll(true);
	geodesic.SetTimerEnabled(true);
	geodesic.SetVerbose(true);
	//geodesic.SetNumberOfThreads(16);
	geodesic.Execute();
#else
	// For debugging
	ApplicationBase app;
	connect(&app, SIGNAL(ProgressUpdateUI(long, QString, int)), this, SLOT(progressUpdateFromApplication(long, QString, int)));
	app.SetUid(uid);
	emit app.EmitProgressUpdateForDebugging();
#endif // ! BUILD_GEODESIC_TRAINING

	ResultFinished(uid);
}