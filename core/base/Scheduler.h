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

#include "DataManager.h"

class Scheduler : public QObject
{
	Q_OBJECT

public:
	// You can add more Data structs to the queue
	// use customFlag if you need to do many things 
	// + a switch statement at overriden threadJob
	typedef struct SchedulerJobData
	{
		std::vector<long> uids;
		std::map< long, std::vector< long > > iids;
		int customFlag = 0;
	} SchedulerJobData;

	Scheduler(QObject *parent = nullptr);
	~Scheduler();

	void AddData(std::shared_ptr<SchedulerJobData> data);
	void SetDataManager(DataManager* dataManager);
	void SetMaxParallelJobs(int maxParallelJobs);

public slots:
	void progressUpdateFromApplication(long uid, QString message, int progress);
	void Start();
	void Stop();

signals:
	void jobFinished(long uid);
	void jobQueued(long uid);
	void updateProgress(long uid, int progress);
	void roundFinished();

protected:
	virtual void ThreadJob(long uid, std::vector<long> &iids, const int customFlag = 0);
	void ResultFinished(long uid);

	DataManager* m_DataManager;
	std::vector< std::shared_ptr<SchedulerJobData> > m_Data;

private:
	void BackgroundCoordinator();

	int m_MaxParallelJobs = 2;
	std::mutex m_Mutex;
	bool m_StopFlag = false; // Signals to not run additional samples
	bool m_CoordinatorRunning = false;
	size_t m_NumberOfUnfishedJobsThisRound;
	std::thread m_BackgroundCoordinator;
};

#endif // ! SCHEDULER_H