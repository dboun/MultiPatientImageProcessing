#include "DefaultScheduler.h"

#include <QDebug>

DefaultScheduler::DefaultScheduler(QObject *parent) : SchedulerBase(parent)
{
	qDebug() << "DefaultScheduler::DefaultScheduler()";
}

void DefaultScheduler::QueueAlgorithm(AlgorithmModuleBase* algorithmModule)
{
	
}

void DefaultScheduler::ClearQueuedAlgorithms()
{

}