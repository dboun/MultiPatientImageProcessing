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

#include "ApplicationGeodesicTrainingSegmentation.h"

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

	Scheduler();

	void Start();

	void Stop();

	void SetData(std::shared_ptr<Data> data);
	void SetMaxParallelJobs(int maxParallelJobs);

public slots:
	void progressUpdateFromApplication(long uid, QString message, int progress);

signals:
	void jobFinished(long uid);
	void updateProgress(long uid, int progress);

private:
	std::shared_ptr<Data> m_Data;
	int m_MaxParallelJobs = 2;
	std::mutex m_Mutex;

	int m_NumberOfUnfishedJobs;
	std::thread m_BackgroundCoordinator;

	void BackgroundCoordinator();

	void ResultFinished(long uid);

	void ThreadJob(long uid, std::vector<std::string> &imagesPaths, std::string &maskPath, std::string &patientDirectoryPath);
};

#endif // SCHEDULER_H
