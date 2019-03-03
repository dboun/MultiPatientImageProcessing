#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "SchedulerBase.h"

class Scheduler : public SchedulerBase
{
public:
	Scheduler(QObject *parent = nullptr);

private:
	void ThreadJob(long uid, std::vector<long> &iids, const int customFlag = 0) override;
};

#endif // ! SCHEDULER_H