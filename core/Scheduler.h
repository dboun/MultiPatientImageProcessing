#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QDebug>
#include <QThread>

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <map>

//#ifdef BUILD_GEODESIC_TRAINING
#include "ApplicationGeodesicTrainingSegmentation.h"
//#endif // ! BUILD_GEODESIC_TRAINING

class Scheduler: public QObject
{
	Q_OBJECT

public:
	typedef struct Data
	{
		std::vector<long> uids;

		std::map< long, std::vector<std::string> > imagesPaths;
		std::map< long, std::string > maskPath;
		std::map< long, std::string > patientDirectoryPath;
		
		std::map< long, std::string > resultPath;
	} Data;

	Scheduler(QObject *parent = nullptr);
	~Scheduler();

	void AddData(std::shared_ptr<Data> data);
	void SetMaxParallelJobs(int maxParallelJobs);

public slots:
	void progressUpdateFromApplication(long uid, QString message, int progress);
	void Start();
	void Stop();

signals:
	void jobFinished(long uid);
	void updateProgress(long uid, int progress);
	void roundFinished();

private:
	void BackgroundCoordinator();

	void ResultFinished(long uid);

	void ThreadJob(long uid, std::vector<std::string> &imagesPaths, std::string &maskPath, std::string &patientDirectoryPath);


	std::vector< std::shared_ptr<Data> > m_Data;
	int m_MaxParallelJobs = 2;
	std::mutex m_Mutex;
	bool m_StopFlag = false; // Signals to not run additional samples
	bool m_CoordinatorRunning = false;

	size_t m_NumberOfUnfishedJobsThisRound;
	std::thread m_BackgroundCoordinator;
};

#endif // SCHEDULER_H
