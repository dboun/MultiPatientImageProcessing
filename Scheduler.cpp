#include "Scheduler.h"

Scheduler::Scheduler()
{
	int threadCount = QThread::idealThreadCount();

	qDebug() << QString("Ideal number of threads: ") << QString(threadCount);
	
	threadCount = (threadCount / 2 + 1 < 2) ? 2 : threadCount / 2 + 1;
	qDebug() << QString("Automatically setting max parallel jobs to: ") << QString(threadCount);
	
	m_MaxParallelJobs = threadCount;
	//m_MaxParallelJobs = 4;
}

void Scheduler::Start()
{
	if (m_Data.get() == nullptr)
	{
		return;
	}

	//emit jobFinished(0);

	m_NumberOfUnfishedJobs = m_Data->uids.size();

	Stop();
	m_BackgroundCoordinator = std::thread(&Scheduler::BackgroundCoordinator, this);
}

void Scheduler::Stop()
{
	// TODO: Maybe do something for child threads?

	if (m_BackgroundCoordinator.joinable()) { m_BackgroundCoordinator.join(); }
}

void Scheduler::SetData(std::shared_ptr<Data> data)
{
	m_Data = data;
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
	qDebug() << QString("Background coordinator started");

	std::vector<std::thread> threads(m_Data->uids.size());
	int counterForThreadsVec = 0;
	int numberOfOpenThreads = 0;
	int oldestOpenThread = 0;

	for (const auto& uid : m_Data->uids)
	{
		qDebug() << QString("(Background coordinator) Trying to run for uid: ") << QString(uid);

		if (numberOfOpenThreads == m_MaxParallelJobs)
		{
			threads[oldestOpenThread].join();
			oldestOpenThread++;
			numberOfOpenThreads--;
		}

		numberOfOpenThreads++;
		m_Data->resultPath[uid] = m_Data->patientDirectoryPath[uid] + std::string("/MPIP_output/labels_res.nii.gz");
		threads[counterForThreadsVec++] = std::thread(&Scheduler::ThreadJob, this,
			uid, std::ref(m_Data->imagesPaths[uid]), std::ref(m_Data->maskPath[uid]), std::ref(m_Data->patientDirectoryPath[uid])
		);
	}

	for (int i = oldestOpenThread; i < m_Data->uids.size(); i++) {
		threads[i].join();
	}
}

void Scheduler::ResultFinished(long uid)
{
	qDebug() << QString("Result finished called");

	std::lock_guard<std::mutex> lg(m_Mutex);
	m_NumberOfUnfishedJobs--;
	emit jobFinished(uid);

	//...?
}

void Scheduler::ThreadJob(long uid, std::vector<std::string> &imagesPaths, std::string &maskPath, std::string &patientDirectoryPath)
{
	qDebug() << QString("Thread started for: ") << QString(uid);

//#ifdef BUILD_GEODESIC_TRAINING
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
//#endif // ! BUILD_GEODESIC_TRAINING

	ResultFinished(uid);
}